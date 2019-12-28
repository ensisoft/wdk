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


#include <GL/glx.h>     // for GLX
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <string>

#include "wdk/system.h"
#include "wdk/utility.h"
#include "wdk/X11/errorhandler.h"
#include "wdk/opengl/context.h"
#include "wdk/opengl/config.h"
#include "wdk/opengl/surface.h"

#define X11_None 0

// GLX_ARB_create_context         
// Accepted as an attribute name in <*attrib_list>:
#define GLX_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB           0x2092
#define GLX_CONTEXT_FLAGS_ARB                   0x2094
#define GLX_CONTEXT_PROFILE_MASK_ARB            0x9126

// Accepted as bits in the attribute value for GLX_CONTEXT_FLAGS_ARB in
// <*attrib_list>:
#define GLX_CONTEXT_DEBUG_BIT_ARB               0x0001
#define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

// Accepted as bits in the attribute value for
// GLX_CONTEXT_PROFILE_MASK_ARB in <*attrib_list>:
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002


// GLX_EXT_create_context_es2_profile
// Accepted as a bit in the attribute value for
// GLX_CONTEXT_PROFILE_MASK_ARB in <*attrib_list>:
#define GLX_CONTEXT_ES_PROFILE_BIT_EXT		0x00000004
#define GLX_CONTEXT_ES2_PROFILE_BIT_EXT		0x00000004
        
namespace wdk
{

struct Context::impl {
    ::Window           temp_window;
    ::GLXWindow        temp_surface;
    ::GLXDrawable      surface; // current surface
    ::GLXContext       context;

    impl(const Config& conf, int major_version, int minor_version, bool debug, Context::Type type) :
        temp_window(0), temp_surface(0), surface(0), context(0)
    {
        // Context creation requires GLX_ARB_create_context extension.
        // if this is not available at runtime then context creation simply fails.
        typedef GLXContext (APIENTRY *glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

        auto glXCreateContextAttribs = reinterpret_cast<glXCreateContextAttribsARBProc>(glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB"));
        if (!glXCreateContextAttribs)
           throw std::runtime_error("cannot create context");

        Display* dpy    = GetNativeDisplayHandle();
        GLXFBConfig fbc = conf.GetNativeHandle();

        if (type == Context::Type::OpenGL_ES)
        {
            // todo: what's the screen number ? 
            const char* extensions_string = glXQueryExtensionsString(dpy, 0); 
            bool GLX_EXT_create_context_es2_profile_supported = false;
            std::stringstream ss(extensions_string);
            std::string extension;
            while (std::getline(ss, extension, ' '))
            {
                if (extension == "GLX_EXT_create_context_es2_profile")
                {
                    GLX_EXT_create_context_es2_profile_supported = true;
                    break;
                }
            }
            if (!GLX_EXT_create_context_es2_profile_supported)
                throw std::runtime_error("cannot create GL ES context. No GLX_EXT_create_context_es2_profile");
        }

        factory<GLXContext> context_factory(dpy);

        GLXContext context = context_factory.create([&](Display* dpy)
        {
            const int FLAGS = debug ?
               GLX_CONTEXT_DEBUG_BIT_ARB : 0;
            const int PROFILE = (type == Context::Type::OpenGL_ES) 
                ? GLX_CONTEXT_ES2_PROFILE_BIT_EXT : GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

            const int attrs[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, major_version,
                GLX_CONTEXT_MINOR_VERSION_ARB, minor_version,
                GLX_CONTEXT_FLAGS_ARB, FLAGS,
                GLX_CONTEXT_PROFILE_MASK_ARB, PROFILE,
                X11_None
            };

            GLXContext c = glXCreateContextAttribs(dpy, fbc, NULL, GL_TRUE, attrs);
            return c;
        });
        if (context_factory.has_error())
            throw std::runtime_error("create context failed");

        auto visual = MakeUniqueHandle(glXGetVisualFromFBConfig(dpy, fbc), XFree);
        if (!visual.get())
            throw std::runtime_error("get visualinfo failed");

        int root = RootWindow(dpy, DefaultScreen(dpy));

        XSetWindowAttributes attr = {};
        attr.colormap = XCreateColormap(dpy, root, visual->visual, AllocNone);

        // glx won't allow us to create any GL objects unless context has been
        // made current with a window (None won't do). However it makes perfect
        // sense for the client code to be able to create GL objects once it has
        // created a context. (consider a case where the client has a window object
        // that carries some GL objects for simple rendering cases). The objects
        // cannot be created because the contex is not current, and context cannot
        // be made current because the window handle doesn't exist yet.
        // this is a "chicken-egg" problem.
        // The solution here is to create a temporary 1x1 px window and make the
        // context current with that window. Once the client makes first call
        // to make_current with the real window handle we swap that in.
        // this is hack, and I'm proud. (;
        ::Window tmp_window = XCreateWindow(
            dpy,
            root,
            0, 0,
            1, 1,
            0,
            visual->depth,
            InputOutput,
            visual->visual,
            CWColormap,
            &attr);
        GLXWindow tmp_surface = glXCreateWindow(dpy, fbc, tmp_window, NULL);

        if (!glXMakeCurrent(dpy, tmp_surface, context))
            throw std::runtime_error("make current failed");

        this->temp_window  = tmp_window;
        this->temp_surface = tmp_surface;
        this->context      = context;
    }
};

Context::Context(const Config& conf)
{
    pimpl_.reset(new impl(conf, 3, 0, false, Type::OpenGL));
}

Context::Context(const Config& conf, int major_version, int minor_version, bool debug)
{
    pimpl_.reset(new impl(conf, major_version, minor_version, debug, Type::OpenGL));
}

Context::Context(const Config& conf, int major_version, int minor_version, bool debug, Type requested_type) 
{
    pimpl_.reset(new impl(conf, major_version, minor_version, debug, requested_type));
}

Context::~Context()
{
    Display* d = GetNativeDisplayHandle();

    glXMakeCurrent(d, X11_None, NULL);
    glXDestroyContext(d, pimpl_->context);
    glXDestroyWindow(d, pimpl_->temp_surface);
    XDestroyWindow(d, pimpl_->temp_window);
}

void Context::MakeCurrent(Surface* surf)
{
    Display* d = GetNativeDisplayHandle();

    // glXMakeContextCurrent doesn't like None for surface. (mesa 9.2)
    // so instead of None we use the temporary window surface
    glXMakeCurrent(d, pimpl_->temp_surface, pimpl_->context);

    pimpl_->surface = 0;

    if (surf == nullptr)
        return;

    if (!glXMakeCurrent(d, surf->GetNativeHandle(), pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = surf->GetNativeHandle();
}

void Context::SwapBuffers()
{
    assert(pimpl_->surface && "context has no valid surface. did you forget to call make_current?");

    Display* d = GetNativeDisplayHandle();

    glXSwapBuffers(d, pimpl_->surface);
}

bool Context::HasDRI() const
{
    Display* d = GetNativeDisplayHandle();

    return (glXIsDirect(d, pimpl_->context) == True);
}

void* Context::Resolve(const char* function) const
{
    assert(function && "null function name");

    void* ret = (void*)glXGetProcAddress((GLubyte*)function);
    // According to OpenGL wiki https://www.opengl.org/wiki/Load_OpenGL_Functions
    // some implementations can also return 0x1, 0x2, 0x3 or -1 for "not found" instead
    // of NULL
    if (ret == (void*)0x1 || ret == (void*)0x2 || ret == (void*)0x3 || ret == (void*)-1)
        return nullptr;

    return ret;
}

} // wdk
