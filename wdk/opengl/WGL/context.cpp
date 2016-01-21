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
#include <GL/gl.h>
#include <cassert>
#include <stdexcept>
#include <functional>
#include <vector>
#include <wdk/win32/helpers.h>
#include <wdk/utility.h>
#include "../context.h"
#include "../config.h"
#include "../surface.h"

// http://www.opengl.org/registry/specs/ARB/wgl_create_context.txt
// Accepted as an attribute name in <*attribList>:
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

// Accepted as bits in the attribute value for WGL_CONTEXT_FLAGS in
// <*attribList>:
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

// Accepted as bits in the attribute value for
// WGL_CONTEXT_PROFILE_MASK_ARB in <*attribList>:
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// New errors returned by GetLastError :
#define ERROR_INVALID_VERSION_ARB               0x2095
#define ERROR_INVALID_PROFILE_ARB               0x2096

namespace {
    typedef HGLRC (APIENTRY *wglCreateContextAttribsARBProc)(HDC, HGLRC, const int*);
} // namespace

namespace wdk
{
struct context::impl {
    HGLRC    context;
    HDC      surface;    
    dummywin tmp;

    impl(const config& conf, int major, int minor, bool debug)
    {
        auto wglCreateContextAttribsARB = reinterpret_cast<wglCreateContextAttribsARBProc>(context::resolve("wglCreateContextAttribsARB"));
        if (!wglCreateContextAttribsARB)
            throw std::runtime_error("unable to create context. no wglCreateContextAttribs");

        const int pixelformat = conf.visualid();
        const PIXELFORMATDESCRIPTOR* desc = conf.handle();

        if (!SetPixelFormat(tmp.surface(), pixelformat, desc))
            throw std::runtime_error("failed to set appropriate pixelformat on tmp window");

        const int ARNOLD = 0; // attr list terminator

        const int FLAGS = debug ? 
            WGL_CONTEXT_DEBUG_BIT_ARB : 0;

        const int attrs[] = 
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, major,
            WGL_CONTEXT_MINOR_VERSION_ARB, minor,
            WGL_CONTEXT_FLAGS_ARB, FLAGS,
            ARNOLD
        };
		//auto ctx = make_unique_ptr(wglCreateContext(tmp.surface()), wglDeleteContext);
        auto ctx = make_unique_ptr(wglCreateContextAttribsARB(tmp.surface(), nullptr, attrs), wglDeleteContext);
        if (!ctx)
            throw std::runtime_error("create context failed");

        if (!wglMakeCurrent(tmp.surface(), ctx.get()))
            throw std::runtime_error("make current failed");

        this->context = ctx.release();
        this->surface = nullptr;
    }
};

context::context(const config& conf)
{
    pimpl_.reset(new impl(conf, 3, 0, false));
}

context::context(const config& conf, int major_version, int minor_version, bool debug)
{
    pimpl_.reset(new impl(conf, major_version, minor_version, debug));
}

context::~context()
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(pimpl_->context);
}

void context::make_current(surface* surf)
{
    // wgl has a problem similar to glX that you can't pass NULL for HDC.
    // so we use the temporary window surface
    wglMakeCurrent(pimpl_->tmp.surface(), pimpl_->context);

	pimpl_->surface = NULL;

    if (surf == nullptr)
        return;

    if (!wglMakeCurrent(surf->handle(), pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = surf->handle();
}

void context::swap_buffers()
{
    assert(pimpl_->surface && "context has no valid surface. did you forget to call make_current?");
    
    const BOOL ret = SwapBuffers(pimpl_->surface);

    assert(ret == TRUE);
}

bool context::has_dri() const
{
    return true;
}

void* context::resolve(const char* function)
{
    assert(function && "null function name");
    
    if (!wglGetCurrentContext())
    {
        // wglGetProcAddress won't work unless there's a current context
        // so we have a special dummy context created with a dummy window
        // and made current just so that we can query for functions
        // with wglGetProcAddress.  (nice huh??)
        // note that creating a compatible (empty) DC with CreateCompatibleDC
        // wont work but will crash in makeCurrent (nvidia)
        struct dummy_context 
        {
            dummy_context() : hgl(nullptr)
            {
                // have to set the pixel format before wglCreateContext
                // can deal with the HDC.
                PIXELFORMATDESCRIPTOR desc = {0};
                desc.nVersion   = 1;
                desc.dwFlags    = PFD_SUPPORT_OPENGL;

                const int pixelformat = ChoosePixelFormat(win.surface(), &desc);
                if (!SetPixelFormat(win.surface(), pixelformat, &desc))
                    throw std::runtime_error("horrors");

                hgl = wglCreateContext(win.surface());
                if (!hgl)
                    throw std::runtime_error("horrors again");
            }
           ~dummy_context()
            {              
                wglMakeCurrent(NULL, NULL);

                BOOL ret = wglDeleteContext(hgl);
                assert(ret);
            }
            void make_current()
            {
                BOOL ret = wglMakeCurrent(win.surface(), hgl);
                assert(ret);
            }
        private:
            dummywin win;
            HGLRC hgl;
        };

        static dummy_context dummy;

        dummy.make_current();
    }
    
    void* ret = (void*)wglGetProcAddress(function);
    
    return ret;
}

} // wdk
