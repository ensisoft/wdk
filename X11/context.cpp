
#include <GL/glx.h>     // for GLX
#include <X11/Xutil.h> // for glXChooseVisual
#include <memory>
#include <cassert>
#include <stdexcept>
#include "context.h"

namespace wdk
{

struct context::impl {
    native_display_t disp;
    native_window_t  window;
    native_window_t  temp;
    GLXContext       context;
    uint_t           visualid;
};

context::context(native_display_t disp)
{
    // other possible attributes,
    // GLX_X_RENDERABLE, GLX_X_DRAWABLE_TYPE, GLX_WINDOW_BIT, ??

    const int_t attrs[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE,       8,
        GLX_GREEN_SIZE,     8,
        GLX_BLUE_SIZE,      8,
        GLX_ALPHA_SIZE,     8,
        GLX_DEPTH_SIZE,     16,
        None
    };

    init(disp, attrs);
}

context::context(native_display_t disp, const int_t* attrs)
{
    init(disp, attrs);
}

context::~context()
{
    assert(pimpl_->disp);
    assert(pimpl_->context);

    Bool ret = glXMakeCurrent(pimpl_->disp, None, NULL);
    assert(ret == True);
    glXDestroyContext(pimpl_->disp, pimpl_->context);

    if (pimpl_->temp != wdk::NULL_WINDOW)
        XDestroyWindow(pimpl_->disp, pimpl_->temp);
}

void context::make_current(native_window_t window)
{      
    assert(pimpl_->disp);
    assert(pimpl_->context);

    if (window == wdk::NULL_WINDOW)
    {
        if (!glXMakeCurrent(pimpl_->disp, None, NULL))
            throw std::runtime_error("make current failed");
    }
    else
    {
        if (!glXMakeCurrent(pimpl_->disp, window, pimpl_->context))
            throw std::runtime_error("make current failed");
    }

    pimpl_->window = window;

    if (pimpl_->temp != wdk::NULL_WINDOW)
    {
        XDestroyWindow(pimpl_->disp, pimpl_->temp);
        pimpl_->temp = wdk::NULL_WINDOW;
    }
    
}

void context::swap_buffers()
{
    assert(pimpl_->window != wdk::NULL_WINDOW  && "context has no valid surface/window. did you forget to call make_current?");

    glXSwapBuffers(pimpl_->disp, pimpl_->window);
}

uint_t context::visualid() const
{
    return pimpl_->visualid;
}

bool context::has_dri() const
{
    return (glXIsDirect(pimpl_->disp, pimpl_->context) == True);
}

void* context::resolve(const char* function)
{
    assert(function && "null function name");
    
    void* ret = (void*)glXGetProcAddress((GLubyte*)function);
    
    return ret;
}

void context::init(native_display_t disp, const int_t* attrs)
{
    assert(attrs);
    assert(disp);

    pimpl_.reset(new impl);
    pimpl_->disp   = disp;
    pimpl_->window = wdk::NULL_WINDOW;
    
    // todo: how does DefaultScreen work in case of dualhead?
    const int screen  = DefaultScreen(disp);
    const Window root = RootWindow(disp, screen);

    auto visual = make_unique_ptr(glXChooseVisual(disp, screen, (int*)attrs), XFree);
    if (!visual.get())
        throw std::runtime_error("get visual info failed");

    XSetWindowAttributes attr = {};
    attr.colormap = XCreateColormap(disp, root, visual->visual, AllocNone);

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
    pimpl_->temp = XCreateWindow(
       disp,
       root,
       0, 0,
       1, 1,
       0,
       visual->depth,
       InputOutput,
       visual->visual,
       CWColormap,
       &attr);
    if (!pimpl_->temp)
        throw std::runtime_error("create temp window failed");

    // todo: possible context attributes, but GL3 seems to be working without these..
    // const int v3[] = {
    //     GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    //     GLX_CONTEXT_MINOR_VERSION_ARB, 1
    // };
    // glXCreateContextAttribs....
    
    pimpl_->context = glXCreateContext(disp, visual.get(), NULL, GL_TRUE);
    if (!pimpl_->context)
    {
        XDestroyWindow(disp, pimpl_->temp);
        throw std::runtime_error("create context failed");
    }

    glXMakeCurrent(disp, pimpl_->temp, pimpl_->context);

    pimpl_->visualid = visual->visualid;
}

} // wdk
