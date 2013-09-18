

#include <EGL/egl.h>
#include <cassert>
#include <stdexcept>
#include <vector>
#include "../context.h"
#include "../types.h"
#include "../utility.h"

namespace wdk
{

struct context::impl {
    EGLContext      context;
    EGLConfig       config;
    EGLDisplay      display;
    EGLSurface      surface;
    EGLint          visualid;
    native_window_t window;
    native_pixmap_t pixmap;

    void release_surface()
    {
        if (surface == EGL_NO_SURFACE)
            return;

        eglDestroySurface(display, surface);

        surface = EGL_NO_SURFACE;
    }
};

context::attributes::attributes() : 
    major_version(2),
    minor_version(0),
    red_size(8),
    green_size(8),
    blue_size(8),
    alpha_size(8),
    depth_size(8),
    render_window(true),
    render_pixmap(false),
    doublebuffer(true)
{
}

context::context(native_display_t disp, const attributes& attrs)
{
    assert(disp);

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

    std::vector<uint_t> window_config = 
    {
        EGL_RED_SIZE, attrs.red_size,
        EGL_GREEN_SIZE, attrs.green_size,
        EGL_BLUE_SIZE, attrs.blue_size,
        EGL_ALPHA_SIZE, attrs.alpha_size,
        EGL_DEPTH_SIZE, attrs.depth_size,
        EGL_CONFIG_ID, attrs.visualid,
        EGL_NONE
    };

    uint_t drawable_type_bits = 0;
    if (attrs.render_window)
        drawable_type_bits |= EGL_WINDOW_BIT;
    if (attrs.render_pixmap)
        drawable_type_bits |= EGL_PIXMAP_BIT;

    window_config.push_back(EGL_SURFACE_TYPE);
    window_config.push_back(drawable_type_bits);
    window_config.push_back(EGL_NONE);

    EGLint numconf   = 0;
    EGLConfig config = NULL;
    if (!eglChooseConfig(pimpl_->display, (const EGLint*)&window_config[0], &config, 1, &numconf))
        throw std::runtime_error("no matching config");
    
    EGLint visualid  = 0;
    if (!eglGetConfigAttrib(pimpl_->display, config, EGL_NATIVE_VISUAL_ID, &visualid))
        throw std::runtime_error("no visualid");
    
    const EGLint context_config[] = 
    {
        EGL_CONTEXT_CLIENT_VERSION, (EGLint)attrs.major_version,
        EGL_NONE
    };

    pimpl_->context = eglCreateContext(pimpl_->display, config, EGL_NO_CONTEXT, context_config); // share_context
    if (!pimpl_->context)
        throw std::runtime_error("create context failed");
    
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);

    pimpl_->visualid = visualid;
    pimpl_->config   = config;

}


context::~context()
{
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    pimpl_->release_surface();

    eglDestroyContext(pimpl_->display, pimpl_->context);

    eglTerminate(pimpl_->display);
}

void context::make_current(native_window_t window)
{
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);

    pimpl_->release_surface();

    if (window == wdk::NULL_WINDOW)
        return;

    EGLSurface surface = eglCreateWindowSurface(pimpl_->display, pimpl_->config, window, NULL);
    if (surface == EGL_NO_SURFACE)
        throw std::runtime_error("create window surface failed");

    if (!eglMakeCurrent(pimpl_->display, surface, surface, pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = surface;
}

void context::make_current(native_pixmap_t pixmap)
{
    eglMakeCurrent(pimpl_->display, EGL_NO_SURFACE, EGL_NO_SURFACE, pimpl_->context);

    pimpl_->release_surface();

    if (pixmap == wdk::NULL_PIXMAP)
        return;

    EGLSurface surface = eglCreatePixmapSurface(pimpl_->display, pimpl_->config, pixmap, NULL);
    if (surface == EGL_NO_SURFACE)
        throw std::runtime_error("create pixmap surface failed");

    if (!eglMakeCurrent(pimpl_->display, surface, surface, pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = surface;
}

void context::swap_buffers()
{
    assert((pimpl_->surface != EGL_NO_SURFACE) && "context has no valid surface. did you forget to call make_current?");
    
    EGLBoolean ret = eglSwapBuffers(pimpl_->display, pimpl_->surface);

    assert(ret);
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

} // wdk
