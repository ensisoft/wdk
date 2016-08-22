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
#include <wdk/window.h>
#include <wdk/pixmap.h>
#include "../surface.h"
#include "../config.h"
#include "fakecontext.h"

namespace {
    enum class surface_type { window };
} // namespace

namespace wdk
{
struct surface::impl {
    HDC hdc;
    surface_type type;
    uint_t width;
    uint_t height;
};

surface::surface(const config& conf, const window& win)
{
    pimpl_.reset(new impl);
    pimpl_->hdc    = GetDC(win.handle());
    pimpl_->width  = 0;
    pimpl_->height = 0;
    pimpl_->type   = surface_type::window;

    // grab the pixelformat descriptor that the configuration has
    // and then resolve that to the HDC specific pixel format index.
    const PIXELFORMATDESCRIPTOR* desc = conf.handle();

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
        ReleaseDC(win.handle(), pimpl_->hdc);
        throw std::runtime_error("set pixelformat failed");
    }
}

surface::surface(const config& conf, const pixmap& px)
{
    assert(!"not supported");
}

surface::surface(const config& conf, uint_t width, uint_t height)
{
    assert(!"not implemented");
}

surface::~surface()
{
    dispose();
}

gl_surface_t surface::handle() const
{
    return pimpl_->hdc;
}

uint_t surface::width() const
{
    if (pimpl_->type != surface_type::window)
        return pimpl_->width;

    HWND hwnd = WindowFromDC(pimpl_->hdc);
    RECT rc = {0};
    GetClientRect(hwnd, &rc);

    return rc.right;
}

uint_t surface::height() const
{
    if (pimpl_->type != surface_type::window)
        return pimpl_->height;

    HWND hwnd = WindowFromDC(pimpl_->hdc);
    RECT rc = {0};
    GetClientRect(hwnd, &rc);

    return rc.bottom;
}

void surface::dispose()
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
    }
    pimpl_->hdc = NULL;
}

} // wdk