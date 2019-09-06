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
#include <wdk/system.h>
#include <cassert>
#include <stdexcept>
#include <vector>
#include "../context.h"
#include "../types.h"
#include "../config.h"
#include "../surface.h"
#include "egldisplay.h"

// http://www.khronos.org/registry/egl/extensions/KHR/EGL_KHR_create_context.txt

// Accepted as an attribute name in the <*attrib_list> argument of
// eglCreateContext:
#define EGL_CONTEXT_MAJOR_VERSION_KHR           0x3098 // (this token is an alias for EGL_CONTEXT_CLIENT_VERSION)
#define EGL_CONTEXT_MINOR_VERSION_KHR           0x30FB
#define EGL_CONTEXT_FLAGS_KHR                   0x30FC
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR     0x30FD
#define EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR  0x31BD

// Accepted as a bitfield value in the EGL_RENDERABLE_TYPE config
// attribute to eglChooseConfig:
#define EGL_OPENGL_ES3_BIT_KHR                  0x0040

// Accepted as attribute values for
// EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR:
#define EGL_NO_RESET_NOTIFICATION_KHR           0x31BE
#define EGL_LOSE_CONTEXT_ON_RESET_KHR           0x31BF

// Accepted as bits in the attribute value for EGL_CONTEXT_FLAGS_KHR in
// <*attrib_list>:
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR               0x00000001
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR  0x00000002
#define EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR       0x00000004

namespace wdk
{

struct context::impl {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    impl(const wdk::config& conf, int major_version, int minor_version, bool debug) :
        display(nullptr), surface(nullptr), context(nullptr)
    {
        display = egl_init(get_display_handle());

        const EGLint FLAGS = debug ?
            EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR : 0;

        // we require EGL_KHR_create_context extension for the debug context
        // if this extension is not available at runtime context creation
        // will simply fail.
        const EGLint attrs[] = {
            EGL_CONTEXT_CLIENT_VERSION, (EGLint)major_version,
            EGL_CONTEXT_FLAGS_KHR, FLAGS,
            EGL_NONE
        };

        // Theoretically we shouldn't hardcode the API here.
        // It's conceivable that the user of this library would want to use
        // the EGL framework to setup rendering context for other apis such as GL or VG.
        // In order to support this properly the config selection needs to support
        // choosing configs which support these rendering apis
        // and the context creation needs an API as well.
        // But for the time being we're just going to baldly assume the user always
        // wants to use OpenGL ES.
        const auto BeforeAPI = eglQueryAPI();

        // force switch
        eglBindAPI(EGL_OPENGL_ES_API);

        context = eglCreateContext(display, conf.handle(), EGL_NO_CONTEXT, attrs);
        if (!context)
            throw std::runtime_error("create context failed");

        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);

        eglBindAPI(BeforeAPI);
    }
};

context::context(const config& conf)
{
    pimpl_.reset(new impl(conf, 2, 0, false));
}

context::context(const config& conf, int major_version, int minor_version, bool debug)
{
    pimpl_.reset(new impl(conf, major_version, minor_version, debug));
}

context::context(const config& conf, int major_version, int minor_version, bool debug, type requested_type) 
{
    // currently not supported.
    if (requested_type == context::type::desktop)
        throw std::runtime_error("not supported");
    pimpl_.reset(new impl(conf, major_version, minor_version, debug));
}

context::~context()
{
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(pimpl_->display, pimpl_->context);
}

void context::make_current(surface* surf)
{
    // See comments about this BindAPI call in the context::impl constructor.
    eglBindAPI(EGL_OPENGL_ES_API);

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

void* context::resolve(const char* function) const
{
    assert(function && "null function name");

    void* ret = (void*)eglGetProcAddress(function);

    return ret;
}

} // wdk
