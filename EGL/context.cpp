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
#include <cassert>
#include <stdexcept>
#include <vector>
#include "../context.h"
#include "../types.h"
#include "../display.h"
#include "../config.h"
#include "../surface.h"

namespace wdk
{

struct context::impl {
    EGLDisplay      display;
    EGLSurface      surface;
    EGLContext      context;

    impl(const wdk::display& disp, const wdk::config& conf, int major_version, int minor_version) :
    display(nullptr),
    surface(nullptr),
    context(nullptr)
    {
        const EGLint attrs[] = 
        {
            EGL_CONTEXT_CLIENT_VERSION, (EGLint)major_version,
            EGL_NONE
        };
#if defined(WINDOWS) || defined(_WIN32)
        display = eglGetdisplay(EGL_DEFAULT_DISPLAY);
#else
        display = eglGetDisplay(disp.handle());
#endif
        if (!display)
            throw std::runtime_error("get EGL display failed");

        // todo: factor the eglGetDisplay, eglInit and eglTerminate into a single place (see config.cpp)

        context = eglCreateContext(display, conf.handle(), EGL_NO_CONTEXT, attrs);
        if (!context)
            throw std::runtime_error("create context failed");
    }
};

context::context(const display& disp, const config& conf)
{
    pimpl_.reset(new impl(disp, conf, 2, 0));
}

context::context(const display& disp, const config& conf, int major_version, int minor_version)
{
    pimpl_.reset(new impl(disp, conf, major_version, minor_version));
}


context::~context()
{
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(pimpl_->display, pimpl_->context);
}

void context::make_current(surface* surf)
{
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);

    pimpl_->surface = nullptr;

    if (surf == nullptr)
        return;

    if (!eglMakeCurrent(pimpl_->display, surf->handle(), surf->handle(), pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = surf->handle();
}

void context::swap_buffers()
{
    assert((pimpl_->surface != EGL_NO_SURFACE) && "context has no valid surface. did you forget to call make_current?");
    
    EGLBoolean ret = eglSwapBuffers(pimpl_->display, pimpl_->surface);

    assert(ret);
}

bool context::has_dri() const
{
    // todo ???
    return true; 
}

void* context::resolve(const char* function)
{
    assert(function && "null function name");
    
    void* ret = (void*)eglGetProcAddress(function);
    
    return ret;
}

} // wdk
