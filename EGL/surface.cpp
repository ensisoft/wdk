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

#include <EGL/egl.h>
#include "../surface.h"
#include "../display.h"
#include "../window.h"
#include "../pixmap.h"
#include "../config.h"

namespace wdk 
{
struct surface::impl {
    EGLDisplay display;
    EGLSurface surface;
    uint_t     width;
    uint_t     height;
};

surface::surface(const display& disp, const config& conf, const window& win) : pimpl_(new impl)
{
#if defined(WINDOWS) || defined(_WIN32)
    // todo: what should the display handle be?
    pimpl_->display = eglGetDisplay(EGL_DEFAULT_DISPLAY); // eglGetDisplay(disp);
#else
    pimpl_->display = eglGetDisplay(disp.handle());
#endif
    if (!pimpl_->display)
        throw std::runtime_error("get EGL display failed");        

    pimpl_->surface = eglCreateWindowSurface(pimpl_->display, conf.handle(), win.handle(), nullptr);
    if (!pimpl_->surface)
        throw std::runtime_error("create window surface failed");

    pimpl_->width  = win.surface_width();
    pimpl_->height = win.surface_height();
}

surface::surface(const display& disp, const config& conf, const pixmap& px) : pimpl_(new impl)
{
    // todo:
}

surface::surface(const display& disp, const config& conf, uint_t width, uint_t height) : pimpl_(new impl)
{
    // todo: refactor this
#if defined(WINDOWS) || defined(_WIN32)
    // todo: what should the display handle be?
    pimpl_->display = eglGetDisplay(EGL_DEFAULT_DISPLAY); // eglGetDisplay(disp);
#else
    pimpl_->display = eglGetDisplay(disp.handle());
#endif
    if (!pimpl_->display)
        throw std::runtime_error("get EGL display failed");        

    pimpl_->surface = eglCreatePbufferSurface(pimpl_->display, conf.handle(), nullptr);
    if (!pimpl_->surface)
        throw std::runtime_error("create offscreen surface failed");

    pimpl_->width = width;
    pimpl_->height = height;
}

surface::~surface()
{
    dispose();
}

uint_t surface::width() const
{
    return pimpl_->width;
}

uint_t surface::height() const
{
    return pimpl_->height;
}

gl_surface_t surface::handle() const
{
    return gl_surface_t { pimpl_->surface };
}

void surface::dispose()
{
    if (pimpl_->surface != EGL_NO_SURFACE)
    {
        eglDestroySurface(pimpl_->display, pimpl_->surface);
        pimpl_->surface = EGL_NO_SURFACE;
    }
}

} // wdk
