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

#include <stdexcept>

#include "wdk/system.h"
#include "wdk/window.h"
#include "wdk/pixmap.h"
#include "wdk/opengl/surface.h"
#include "wdk/opengl/config.h"
#include "egldisplay.h"

// EGK_KHR_gl_colorspace.txt
//
// Accepted as an attribute name by eglCreateWindowSurface,
// eglCreatePbufferSurface and eglCreatePixmapSurface

#define EGL_GL_COLORSPACE_KHR                   0x309D

// Accepted as attribute values for EGL_GL_COLORSPACE_KHR by
// eglCreateWindowSurface, eglCreatePbufferSurface and
// eglCreatePixmapSurface
#define EGL_GL_COLORSPACE_SRGB_KHR              0x3089
#define EGL_GL_COLORSPACE_LINEAR_KHR            0x308A


namespace wdk 
{
struct Surface::impl {
    EGLDisplay display;
    EGLSurface surface;
};

Surface::Surface(const Config& conf, const Window& win) : pimpl_(new impl)
{
    pimpl_->display = egl_init(GetNativeDisplayHandle());

    std::vector<EGLint> attribs;
    if (conf.sRGB())
    { 
        attribs.push_back(EGL_GL_COLORSPACE_KHR);
        attribs.push_back(EGL_GL_COLORSPACE_SRGB_KHR);
    }
    attribs.push_back(EGL_NONE);

    pimpl_->surface = eglCreateWindowSurface(pimpl_->display, conf.GetNativeHandle(), win.GetNativeHandle(), &attribs[0]);
    if (!pimpl_->surface)
        throw std::runtime_error("create window surface failed");
}

Surface::Surface(const Config& conf, const Pixmap& px) : pimpl_(new impl)
{
    pimpl_->display = egl_init(GetNativeDisplayHandle());

    std::vector<EGLint> attribs;
    if (conf.sRGB())
    {
        attribs.push_back(EGL_GL_COLORSPACE_KHR);
        attribs.push_back(EGL_GL_COLORSPACE_SRGB_KHR);
    }
    attribs.push_back(EGL_NONE);

    pimpl_->surface = eglCreatePixmapSurface(pimpl_->display, 
        conf.GetNativeHandle(), px.GetNativeHandle(), &attribs[0]);
    if (!pimpl_->surface)
        throw std::runtime_error("create pixmap surface failed");
}

Surface::Surface(const Config& conf, uint_t width, uint_t height) : pimpl_(new impl)
{
    pimpl_->display = egl_init(GetNativeDisplayHandle());

    std::vector<EGLint> attribs {
        EGL_HEIGHT, (EGLint)height,
	EGL_WIDTH,  (EGLint)width
    };

    if (conf.sRGB())
    {
        attribs.push_back(EGL_GL_COLORSPACE_KHR);
        attribs.push_back(EGL_GL_COLORSPACE_SRGB_KHR);
    } 
    attribs.push_back(EGL_NONE);

    pimpl_->surface = eglCreatePbufferSurface(pimpl_->display, conf.GetNativeHandle(), &attribs[0]);
    if (!pimpl_->surface)
        throw std::runtime_error("create offscreen surface failed");
}

Surface::~Surface()
{
    Dispose();
}

uint_t Surface::GetWidth() const
{
    EGLint width = 0;

    eglQuerySurface(pimpl_->display, pimpl_->surface, EGL_WIDTH, &width);

    return (uint_t)width;
}

uint_t Surface::GetHeight() const
{
    EGLint height = 0;

    eglQuerySurface(pimpl_->display, pimpl_->surface, EGL_HEIGHT, &height);

    return (uint_t)height;
}

gl_surface_t Surface::GetNativeHandle() const
{
    return gl_surface_t { pimpl_->surface };
}

void Surface::Dispose()
{
    if (pimpl_->surface != EGL_NO_SURFACE)
    {
        eglDestroySurface(pimpl_->display, pimpl_->surface);
        pimpl_->surface = EGL_NO_SURFACE;
    }
}

} // wdk
