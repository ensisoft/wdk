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
#include <wdk/utility.h>
#include "../context.h"
#include "../config.h"
#include "../surface.h"
#include "fakecontext.h"

#pragma comment(lib, "opengl32.lib") // needed for wgl functions

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
    std::shared_ptr<wgl::FakeContext> fake;

    HGLRC    context;
    HDC      surface;    

    impl(const config& conf, int major, int minor, bool debug)
    {
        // when config was created it has created a fake gl context.
        // we'll need to retrive that context now to query for the "real" 
        // context creation functions.
        // also we'll use the fake window hdc (that the context has created)
        // untill the the user sets the real surface to this context.
        wgl::fetchFakeContext(&conf, fake);
        assert(fake); 

        auto wglCreateContextAttribsARB = fake->resolve<wglCreateContextAttribsARBProc>("wglCreateContextAttribsARB");
        if (!wglCreateContextAttribsARB)
            throw std::runtime_error("unable to create context. no wglCreateContextAttribs");

        const int ARNOLD = 0; // attr list terminator

        const int FLAGS = debug ? 
            WGL_CONTEXT_DEBUG_BIT_ARB : 0;
            // todo: the context profile bit
        const int PROFILE =  0; // WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            //WGL_CONTEXT_CORE_PROFILE_BIT_ARB;

        const int attrs[] = 
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, major,
            WGL_CONTEXT_MINOR_VERSION_ARB, minor,
            WGL_CONTEXT_FLAGS_ARB, FLAGS,
            //WGL_CONTEXT_PROFILE_MASK_ARB, PROFILE, 
            ARNOLD
        };
        auto ctx = make_unique_ptr(wglCreateContextAttribsARB(fake->getDC(), nullptr, attrs), wglDeleteContext);
        if (!ctx)
            throw std::runtime_error("create context failed");

        if (!wglMakeCurrent(fake->getDC(), ctx.get()))
            throw std::runtime_error("make current failed");

        this->context = ctx.release();
        this->surface = fake->getDC();
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
    const auto hgl = wglGetCurrentContext();
    if (hgl == pimpl_->context)
        wglMakeCurrent(NULL, NULL);

    wglDeleteContext(pimpl_->context);
}

void context::make_current(surface* surf)
{
    // wgl has a problem similar to glX that you can't pass NULL for HDC.
    // so we use the temporary window surface
    if (!surf)
    {
        wglMakeCurrent(pimpl_->fake->getDC(), pimpl_->context);
        pimpl_->surface = pimpl_->fake->getDC();
    }
    else
    {
        if (!wglMakeCurrent(surf->handle(), pimpl_->context))
        {
            if (GetLastError() == ERROR_INVALID_PIXEL_FORMAT)
                throw std::runtime_error("make current failed. "
                    "surface doesn't have a pixel format compatible with the contex");
            throw std::runtime_error("make current failed");
        }                
        pimpl_->surface = surf->handle();
    }
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

void* context::resolve(const char* function) const
{
    assert(function && "null function name");
    
    // WGL proc addressess are per context and can theoretically
    // change between different context instances.
    // According to MSDN they're also unique for each pixel format.
    const auto hdc = wglGetCurrentDC();
    const auto hgl = wglGetCurrentContext();

    if (hdc != pimpl_->surface && hgl != pimpl_->context)
    {
        if (!wglMakeCurrent(pimpl_->surface, pimpl_->context))
            throw std::runtime_error("failed to make context current for proc address query");
    } 
    
    void* ret = (void*)wglGetProcAddress(function);
    if (hdc != pimpl_->surface && hgl != pimpl_->context)
    {
        if (!wglMakeCurrent(hdc, hgl))
            throw std::runtime_error("failed to restore previous context");
    }

    if (ret) return ret;

    // There's the issue that according to the MSDN wglGetProcAddress 
    // returns *extension* function addresses. Original OpenGL (1.x) functions
    // such as glClear are exported directly by Microsoft's OpenGL implementation. 
    // Now different drivers seem to have different behaviour, nvidia (currently 372.54)
    // does return function pointers for these but Intel doesn't. 
    const static auto opengl32 = LoadLibraryA("opengl32.dll");
    ret = GetProcAddress(opengl32, function); 

    // According to OpenGL wiki https://www.opengl.org/wiki/Load_OpenGL_Functions
    // some implementations can also return 0x1, 0x2, 0x3 or -1 for "not found" instead
    // of NULL
    if (ret == (void*)0x1 || ret == (void*)0x2 || ret == (void*)0x3 || ret == (void*)-1)
        return nullptr;
    
    return ret;
}

} // wdk
