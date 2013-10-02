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
#include "../context.h"
#include "../config.h"
#include "../system.h"
#include "../surface.h"
#include "../X11/errorhandler.h"

namespace wdk
{

struct context::impl {
    Window           temp_window;
    GLXWindow        temp_surface;
    GLXDrawable      surface; // current surface
    GLXContext       context;

    impl(const config& conf, int major_version, int minor_version) :
        temp_window(0), temp_surface(0), surface(0), context(0)
    {
        typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

        auto glXCreateContextAttribs = reinterpret_cast<glXCreateContextAttribsARBProc>(context::resolve("glXCreateContextAttribsARB"));
        if (!glXCreateContextAttribs)
           throw std::runtime_error("cannot create context");

        
        Display* dpy    = get_display_handle();
        GLXFBConfig fbc = conf.handle();

        factory<GLXContext> context_factory(dpy);

        GLXContext context = context_factory.create([&](Display* dpy)
        {
            const int attrs[] = 
            {
                GLX_CONTEXT_MAJOR_VERSION_ARB, major_version,
                GLX_CONTEXT_MINOR_VERSION_ARB, minor_version,
                None
            };
            GLXContext c = glXCreateContextAttribs(dpy, fbc, NULL, GL_TRUE, attrs);
            return c;
        });
        if (context_factory.has_error())
            throw std::runtime_error("create context failed");

        auto visual = make_unique_ptr(glXGetVisualFromFBConfig(dpy, fbc), XFree);
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
        Window tmp_window = XCreateWindow(
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

context::context(const config& conf)
{
    pimpl_.reset(new impl(conf, 3, 0));
}

context::context(const config& conf, int major_version, int minor_version)
{
    pimpl_.reset(new impl(conf, major_version, minor_version));
}

context::~context()
{
    Display* d = get_display_handle();

    glXMakeCurrent(d, None, NULL);

    glXDestroyContext(d, pimpl_->context);

    glXDestroyWindow(d, pimpl_->temp_surface);
    XDestroyWindow(d, pimpl_->temp_window);
}

void context::make_current(surface* surf)
{
    Display* d = get_display_handle();

    // glXMakeContextCurrent doesn't like None for surface. (mesa 9.2)
    // so instead of None we use the temporary window surface
    glXMakeCurrent(d, pimpl_->temp_surface, pimpl_->context);

    pimpl_->surface = 0;

    if (surf == nullptr)
        return;

    if (!glXMakeCurrent(d, surf->handle(), pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = surf->handle();
}

void context::swap_buffers()
{
    assert(pimpl_->surface && "context has no valid surface. did you forget to call make_current?");

    Display* d = get_display_handle();

    glXSwapBuffers(d, pimpl_->surface);
}

bool context::has_dri() const
{
    Display* d = get_display_handle();

    return (glXIsDirect(d, pimpl_->context) == True);
}

void* context::resolve(const char* function)
{
    assert(function && "null function name");
    
    void* ret = (void*)glXGetProcAddress((GLubyte*)function);
    
    return ret;
}

} // wdk
