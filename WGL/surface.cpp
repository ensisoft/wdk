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
#include "../surface.h"
#include "../window.h"
#include "../config.h"

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

surface::surface(const display& disp, const config& conf, const window& win)
{
    pimpl_.reset(new impl);
    pimpl_->hdc    = GetDC(win.handle());
    pimpl_->width  = 0;
    pimpl_->height = 0;
    pimpl_->type   = surface_type::window;

    if (win.visualid() == conf.visualid())
        return;

    const int pixelformat = conf.visualid();
    const PIXELFORMATDESCRIPTOR* desc = conf.handle();

    if (!SetPixelFormat(pimpl_->hdc, pixelformat, desc))
    {
        ReleaseDC(win.handle(), pimpl_->hdc);
        throw std::runtime_error("set pixelformat failed");
    }

}

surface::surface(const display& disp, const config& conf, const pixmap& px)
{
    // todo:
}

surface::surface(const display& disp, const config& conf, uint_t width, uint_t height)
{
    // todo:
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