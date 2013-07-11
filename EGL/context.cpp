

#include <EGL/egl.h>
#include <cassert>
#include <stdexcept>
#include "context.h"
#include "types.h"
#include "utility.h"

namespace wdk
{

struct context::impl {
    EGLContext      context;
    EGLConfig       config;
    EGLDisplay      display;
    EGLSurface      surface;
    EGLint          visualid;
    native_window_t window;
};

context::context(native_display_t disp)
{

    // simple default arguments
    const int_t attrs[] = {
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_DEPTH_SIZE, 16,
		EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };
    init(disp, attrs);
}

context::context(native_display_t disp, const int_t* attrs)
{
    init(disp, attrs);
}

context::~context()
{
    assert(pimpl_->display);
    assert(pimpl_->context);

    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (pimpl_->surface != EGL_NO_SURFACE)
        eglDestroySurface(pimpl_->display, pimpl_->surface);

    eglDestroyContext(pimpl_->display, pimpl_->context);

    eglTerminate(pimpl_->display);
}

void context::make_current(native_window_t window)
{
    assert(pimpl_->display);
    assert(pimpl_->context);

    if (window == wdk::NULL_WINDOW)
    {
        if (!eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context))
            throw std::runtime_error("make current failed");
    }
    else
    {
        if (pimpl_->window != window && pimpl_->surface != EGL_NO_SURFACE)
            eglDestroySurface(pimpl_->display, pimpl_->surface);

        pimpl_->surface = eglCreateWindowSurface(pimpl_->display, pimpl_->config, window, NULL);
        if (pimpl_->surface == EGL_NO_SURFACE)
            throw std::runtime_error("create window surface failed");
        
        if (!eglMakeCurrent(pimpl_->display, pimpl_->surface, pimpl_->surface, pimpl_->context))
            throw std::runtime_error("make current failed");
    }
    pimpl_->window = window;
}

void context::swap_buffers()
{
    assert(pimpl_->display);
    assert(pimpl_->context);
    assert((pimpl_->surface != EGL_NO_SURFACE) && "context has no valid surface");
    
    EGLBoolean ret = eglSwapBuffers(pimpl_->display, pimpl_->surface);
    
    int err = eglGetError();

    //assert(ret);
}

uint_t context::visualid() const
{
    return pimpl_->visualid;
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

void context::init(native_display_t disp, const int_t* attrs)
{
    assert(disp);
    assert(attrs);

    pimpl_.reset(new impl);
    pimpl_->config   = NULL;
    pimpl_->surface  = EGL_NO_SURFACE;
    pimpl_->window   = wdk::NULL_WINDOW;
    pimpl_->visualid = 0;
#ifdef WINDOWS
    // todo: what should the display handle be?
    pimpl_->display  = eglGetDisplay(EGL_DEFAULT_DISPLAY); // eglGetDisplay(disp);
#else
    pimpl_->display  = eglGetDisplay(disp);
#endif
    if (!pimpl_->display)
        throw std::runtime_error("get EGL display failed");
    
    EGLint major = 0;
    EGLint minor = 0;
    if (!eglInitialize(pimpl_->display, &major, &minor))
        throw std::runtime_error("EGL initialize failed");

    if (!eglBindAPI(EGL_OPENGL_ES_API))
        throw std::runtime_error("API bind failed");

    EGLint numconf   = 0;
    EGLConfig config = NULL;
    if (!eglChooseConfig(pimpl_->display, attrs, &config, 1, &numconf))
        throw std::runtime_error("no matching config");
    
    EGLint visualid  = 0;
    if (!eglGetConfigAttrib(pimpl_->display, config, EGL_NATIVE_VISUAL_ID, &visualid))
        throw std::runtime_error("no visualid");
    
    const EGLint v2[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    pimpl_->context = eglCreateContext(pimpl_->display, config, EGL_NO_CONTEXT, v2); // share_context
    if (!pimpl_->context)
        throw std::runtime_error("create context failed");
    
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);

    pimpl_->visualid = visualid;
    pimpl_->config   = config;
}

} // wdk
