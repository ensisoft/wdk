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
#include <X11/Xutil.h>

#include <stdexcept>
#include <functional>
#include <cassert>
#include "wdk/pixmap.h"
#include "wdk/system.h"
#include "wdk/X11/errorhandler.h"

namespace wdk
{

struct Pixmap::impl {
    ::Pixmap handle = 0;
    uint_t width = 0;
    uint_t height = 0;
    uint_t depth  = 0;
};

Pixmap::Pixmap(uint_t width, uint_t height, uint_t visualid)
{
    assert(width && height);

    Display* dpy = GetNativeDisplayHandle();

    XVisualInfo vistemplate = {0};
    vistemplate.visualid    = visualid ? visualid : 0;
    const long visual_mask  = visualid ? VisualIDMask : 0;

    int num_visuals = 0;
    XVisualInfo* visinfo = XGetVisualInfo(dpy, visual_mask, &vistemplate, &num_visuals);
    if (!visinfo || !num_visuals)
        throw std::runtime_error("no such visual");


    uint_t bit_depth = visinfo->depth;

    XFree(visinfo);

    factory<::Pixmap> px_factory(dpy);

    ::Pixmap px = px_factory.create(std::bind(XCreatePixmap, std::placeholders::_1, RootWindow(dpy, DefaultScreen(dpy)), width, height, bit_depth));
    if (px_factory.has_error())
        throw std::runtime_error("failed to create pixmap");

    pimpl_.reset(new impl);
    pimpl_->handle = px;
    pimpl_->width  = width;
    pimpl_->height = height;
    pimpl_->depth  = bit_depth;

#ifndef _NDEBUG
    uint_t w, h, d;
    int x, y;
    uint_t border;
    Window dummy;
    XGetGeometry(dpy, px, &dummy, &x, &y, &w, &h, &border, &d);

    assert(w == width);
    assert(h == height);
    //assert(d == bit_depth);
#endif
}

Pixmap::~Pixmap()
{
    Display* d = GetNativeDisplayHandle();

    XFreePixmap(d, pimpl_->handle);
}

native_pixmap_t Pixmap::GetNativeHandle() const
{
    return native_pixmap_t {pimpl_->handle};
}

uint_t Pixmap::GetWidth() const
{
    return pimpl_->width;
}

uint_t Pixmap::GetHeight() const
{
    return pimpl_->height;
}

uint_t Pixmap::GetBitDepth() const
{
    return pimpl_->depth * 8;
}

} // wdk

