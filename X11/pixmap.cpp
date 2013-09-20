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

#include <X11/Xlib.h>
#include <stdexcept>
#include <functional>
#include <cassert>
#include "../pixmap.h"
#include "../display.h"
#include "error_handler.h"

namespace wdk
{

struct pixmap::impl {
    Pixmap handle;
    Display* disp;
    uint_t width;
    uint_t height;
    uint_t depth;
};

pixmap::pixmap(const wdk::display& disp, uint_t width, uint_t height, uint_t visualid)
{
    assert(width && height);

    Display* dpy = disp.handle();

    // todo: visualid??
    uint_t bit_depth = 32;

    factory<Pixmap> px_factory(dpy);

    Pixmap px = px_factory.create(std::bind(XCreatePixmap, std::placeholders::_1, RootWindow(dpy, DefaultScreen(dpy)), width, height, bit_depth));
    if (px_factory.has_error())
        throw std::runtime_error("failed to create pixmap");

    pimpl_.reset(new impl);
    pimpl_->handle = px;
    pimpl_->disp   = dpy;
    pimpl_->width  = width;
    pimpl_->height = height;
    pimpl_->depth  = bit_depth;

#ifndef _NDEBUG
    uint_t w, h, d;
    int x, y;
    uint_t border;
    Window dummy;
    XGetGeometry(pimpl_->disp, px, &dummy, &x, &y, &w, &h, &border, &d);

    assert(w == width);
    assert(h == height);
    //assert(d == bit_depth);
#endif  
}

pixmap::~pixmap()
{
    XFreePixmap(pimpl_->disp, pimpl_->handle);
}

native_pixmap_t pixmap::handle() const
{
    return native_pixmap_t {pimpl_->handle};
}

native_display_t pixmap::display() const
{
    return pimpl_->disp;
}

uint_t pixmap::width() const
{
    return pimpl_->width;
}

uint_t pixmap::height() const
{
    return pimpl_->height;
}

uint_t pixmap::depth() const
{
    return pimpl_->depth;
}

} // wdk

