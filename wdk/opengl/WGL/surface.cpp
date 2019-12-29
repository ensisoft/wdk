// Copyright (c) 2013 Sami Väisänen, Ensisoft 
//
// http://www.ensisoft.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include <windows.h>

#include <cassert>

#include <GL/gl.h>

#include "wdk/window.h"
#include "wdk/pixmap.h"
#include "wdk/opengl/surface.h"
#include "wdk/opengl/config.h"
#include "fakecontext.h"

DECLARE_HANDLE(HPBUFFERARB);

namespace {
    enum class surface_type { window, pbuffer };

    // WGL_ARB_pbuffer
    typedef HPBUFFERARB (APIENTRY *wglCreatePbufferARBProc)(HDC device, 
        int iPixelFormat,
        int iWidth,
        int iHeight, 
        const int* pAttribList);
    // get DC for the pbuffer
    typedef HDC (APIENTRY  *wglGetPbufferDCARBProc)(HPBUFFERARB hPbuffer);
    // release the pbuffer dc
    typedef int (APIENTRY *wglReleasePbufferDCARBProc)(HPBUFFERARB hPbuffer, HDC hDC);
    // destroy pbuffer
    typedef BOOL (APIENTRY *wglDestroyPbufferARBProc)(HPBUFFERARB hPbuffer);
        
} // namespace

namespace wdk
{
struct Surface::impl {
    HPBUFFERARB pbuffer = NULL;
    HDC hdc = NULL;
    surface_type type;
    uint_t width  = 0;
    uint_t height = 0;
    std::shared_ptr<wgl::FakeContext> fake;
};

Surface::Surface(const Config& conf, const Window& win)
{
    pimpl_.reset(new impl);
    pimpl_->hdc    = GetDC(win.GetNativeHandle());
    pimpl_->width  = 0;
    pimpl_->height = 0;
    pimpl_->type   = surface_type::window;

    // grab the pixelformat descriptor that the configuration has
    // and then resolve that to the HDC specific pixel format index.
    const PIXELFORMATDESCRIPTOR* desc = conf.GetNativeHandle();

    // note: this PixelFormat id can be different than what the
    // "visualid" of the config object is since it's per HDC
    const int PixelFormat = ChoosePixelFormat(pimpl_->hdc, desc);

    // the pixelformat can be set only once, so check here instead of 
    // erroring out later.
    const int CurrentPixelFormat = GetPixelFormat(pimpl_->hdc);
    if (CurrentPixelFormat == PixelFormat)
        return;

    // Windows allows us to do this only once!
    if (!SetPixelFormat(pimpl_->hdc, PixelFormat, desc))
    {
        ReleaseDC(win.GetNativeHandle(), pimpl_->hdc);
        throw std::runtime_error("set pixelformat failed");
    }
}

Surface::Surface(const Config& conf, const Pixmap& px)
{
    assert(!"not supported");
}

Surface::Surface(const Config& conf, uint_t width, uint_t height)
{
    std::shared_ptr<wgl::FakeContext> fake;
    wgl::FetchFakeContext(&conf, fake);

    auto wglCreatePbufferARB = fake->Resolve<wglCreatePbufferARBProc>("wglCreatePbufferARB");
    if (!wglCreatePbufferARB)
        throw std::runtime_error("unable to create pbuffer. no wglCreatePbufferARB");
    auto wglGetPbufferDCARB = fake->Resolve<wglGetPbufferDCARBProc>("wglGetPbufferDCARB");
    
    const int attrib_list[1] = {0};

     // grab the pixelformat descriptor that the configuration has
    // and then resolve that to the HDC specific pixel format index.
    const PIXELFORMATDESCRIPTOR* desc = conf.GetNativeHandle();

    const int PixelFormat = ChoosePixelFormat(fake->GetDC(), desc);

    auto pbuff = wglCreatePbufferARB(fake->GetDC(), PixelFormat, width, height, attrib_list);
    if (pbuff == NULL)
        throw std::runtime_error("pbuffer creation failed");
    
    pimpl_.reset(new impl);
    pimpl_->width  = width;
    pimpl_->height = height;
    pimpl_->type   = surface_type::pbuffer;
    pimpl_->hdc    = wglGetPbufferDCARB(pbuff);
    pimpl_->pbuffer = pbuff;
    pimpl_->fake   = fake;
}

Surface::~Surface()
{
    Dispose();
}

gl_surface_t Surface::GetNativeHandle() const
{
    return pimpl_->hdc;
}

uint_t Surface::GetWidth() const
{
    if (pimpl_->type != surface_type::window)
        return pimpl_->width;

    HWND hwnd = WindowFromDC(pimpl_->hdc);
    RECT rc = {0};
    GetClientRect(hwnd, &rc);

    return rc.right;
}

uint_t Surface::GetHeight() const
{
    if (pimpl_->type != surface_type::window)
        return pimpl_->height;

    HWND hwnd = WindowFromDC(pimpl_->hdc);
    RECT rc = {0};
    GetClientRect(hwnd, &rc);

    return rc.bottom;
}

void Surface::Dispose()
{
    if (!pimpl_->hdc)
        return;

    switch (pimpl_->type)
    {
        case surface_type::window:
            {
                HWND hwnd = WindowFromDC(pimpl_->hdc);
                ReleaseDC(hwnd, pimpl_->hdc);
            }
            break;
        case surface_type::pbuffer:
            {
                auto fake = pimpl_->fake;
                auto wglReleasePbufferDCARB = fake->Resolve<wglReleasePbufferDCARBProc>("wglReleasePbufferDCARB");
                auto wglDestroyPbufferARB   = fake->Resolve<wglDestroyPbufferARBProc>("wglDestroyPbufferARB");
                wglReleasePbufferDCARB(pimpl_->pbuffer, pimpl_->hdc);
                wglDestroyPbufferARB(pimpl_->pbuffer);
                pimpl_->pbuffer = NULL;
                pimpl_->hdc   = NULL; 
            }
            break;
    }
    pimpl_->hdc = NULL;
}

} // wdk