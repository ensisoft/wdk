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
#include "egldisplay.h"

namespace wdk 
{
struct surface::impl {
    EGLDisplay display;
    EGLSurface surface;
};

surface::surface(const display& disp, const config& conf, const window& win) : pimpl_(new impl)
{
    pimpl_->display = egl_init(disp.handle());

    pimpl_->surface = eglCreateWindowSurface(pimpl_->display, conf.handle(), win.handle(), nullptr);
    if (!pimpl_->surface)
        throw std::runtime_error("create window surface failed");
}

surface::surface(const display& disp, const config& conf, const pixmap& px) : pimpl_(new impl)
{
    pimpl_->display = egl_init(disp.handle());

    pimpl_->surface = eglCreatePixmapSurface(pimpl_->display, conf.handle(), px.handle(), nullptr);
    if (!pimpl_->surface)
        throw std::runtime_error("create pixmap surface failed");
}

surface::surface(const display& disp, const config& conf, uint_t width, uint_t height) : pimpl_(new impl)
{
    pimpl_->display = egl_init(disp.handle());

	const EGLint attrs[] = {
		EGL_HEIGHT, (EGLint)height,
		EGL_WIDTH,(EGLint)width,
		EGL_NONE
	};

    pimpl_->surface = eglCreatePbufferSurface(pimpl_->display, conf.handle(), attrs);
    if (!pimpl_->surface)
        throw std::runtime_error("create offscreen surface failed");
}

surface::~surface()
{
    dispose();
}

uint_t surface::width() const
{
    EGLint width = 0;

    eglQuerySurface(pimpl_->display, pimpl_->surface, EGL_WIDTH, &width);

    return (uint_t)width;
}

uint_t surface::height() const
{
    EGLint height = 0;

    eglQuerySurface(pimpl_->display, pimpl_->surface, EGL_HEIGHT, &height);

    return (uint_t)height;
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
