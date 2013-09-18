
#include <GL/glx.h>     // for GLX
#include <X11/Xutil.h> // for glXChooseVisual
#include <memory>
#include <vector>
#include <cassert>
#include <stdexcept>
#include "../context.h"
#include "error_handler.h"

namespace {
    enum class drawable_type {
        none, window, pixmap, pbuffer
    };

} // namespace

namespace wdk
{

context::attributes::attributes() : 
    major_version(3),
    minor_version(0),
    red_size(8),
    green_size(8),
    blue_size(8),
    alpha_size(8),
    depth_size(16),
    render_window(true),
    render_pixmap(false),
    doublebuffer(true),
    visualid(0)
{
}

struct context::impl {
    native_display_t disp;
    native_window_t  temp_window;
    GLXWindow        temp_surface;
    GLXDrawable      surface; // either GLXWindow or GLXPixmap or GLXPbuffer
    drawable_type    surface_type;
    GLXContext       context;
    GLXFBConfig*     configs;
    GLXFBConfig      config;
    uint_t           visualid;

    void release_surface() 
    {
        switch (surface_type)
        {
            case drawable_type::window:
                glXDestroyWindow(disp, surface);
                break;
            case drawable_type::pixmap:
                glXDestroyPixmap(disp, surface);
                break;
            case drawable_type::pbuffer:
                glXDestroyPbuffer(disp, surface);
                break;
            default:
                break;
        }
        surface      = 0;
        surface_type = drawable_type::none;
    }
};

context::context(native_display_t disp, const attributes& attrs)
{
    assert(disp);
    assert((attrs.render_window || attrs.render_pixmap) && "no drawable specified (window/pixmap)");

    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

    glXCreateContextAttribsARBProc create_context_proc = reinterpret_cast<glXCreateContextAttribsARBProc>(resolve("glXCreateContextAttribsARB"));
    if (!create_context_proc)
       throw std::runtime_error("no glXCreateContextAttribsARB available");

    std::vector<uint_t> window_config = 
    { 
        GLX_RENDER_TYPE,  GLX_RGBA_BIT, 
        GLX_X_RENDERABLE, True,
        GLX_RED_SIZE,     attrs.red_size,
        GLX_GREEN_SIZE,   attrs.green_size,
        GLX_BLUE_SIZE,    attrs.blue_size,
        GLX_ALPHA_SIZE,   attrs.alpha_size,
        GLX_DEPTH_SIZE,   attrs.depth_size,
        GLX_DOUBLEBUFFER, attrs.doublebuffer
        //GLX_FBCONFIG_ID,  attrs.visualid
    };

    if (attrs.visualid)
    {
        window_config.push_back(GLX_FBCONFIG_ID);
        window_config.push_back(attrs.visualid);
    }

    uint_t drawable_type_bits = 0;
    if (attrs.render_window)
        drawable_type_bits |= GLX_WINDOW_BIT;
    if (attrs.render_pixmap)
        drawable_type_bits |= GLX_PIXMAP_BIT;


    window_config.push_back(GLX_DRAWABLE_TYPE);
    window_config.push_back(drawable_type_bits);
    window_config.push_back(None);

    int num_fb_configs = 0;
    auto fb_configs    = make_unique_ptr(glXChooseFBConfig(disp, DefaultScreen(disp), (int*)&window_config[0], &num_fb_configs), XFree);
    if (!fb_configs.get())
        throw std::runtime_error("no such GL config available");

    const uint_t context_config[] = 
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, attrs.major_version,
        GLX_CONTEXT_MINOR_VERSION_ARB, attrs.minor_version,
        None
    };

    GLXFBConfig config = fb_configs.get()[0];

    factory<GLXContext> context_factory(disp);

    GLXContext context = context_factory.create(
        [&](Display* dpy) 
        {
            GLXContext c = create_context_proc(disp, config, NULL, GL_TRUE, (int*)context_config);
            return c;
        }
    );

    if (context_factory.has_error())
        throw std::runtime_error("create context failed");

    auto visual = make_unique_ptr(glXGetVisualFromFBConfig(disp, config), XFree);
    if (!visual.get())
        throw std::runtime_error("get visualinfo failed");

    int root = RootWindow(disp, DefaultScreen(disp));

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
    Window tmp_window = XCreateWindow(
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
    GLXWindow tmp_surface = glXCreateWindow(disp, config, tmp_window, NULL);

    if (!glXMakeCurrent(disp, tmp_surface, context))
        throw std::runtime_error("make current failed");

    pimpl_.reset(new impl);
    pimpl_->disp            = disp;
    pimpl_->visualid        = visual->visualid;
    pimpl_->context         = context;
    pimpl_->configs         = fb_configs.release();
    pimpl_->config          = config;
    pimpl_->surface         = 0;
    pimpl_->surface_type    = drawable_type::none;
    pimpl_->temp_window.xid = tmp_window;
    pimpl_->temp_surface    = tmp_surface;
}

context::~context()
{
    glXMakeCurrent(pimpl_->disp, None, NULL);

    pimpl_->release_surface();

    glXDestroyContext(pimpl_->disp, pimpl_->context);

    glXDestroyWindow(pimpl_->disp, pimpl_->temp_surface);
    XDestroyWindow(pimpl_->disp, pimpl_->temp_window);

    XFree(pimpl_->configs);
}

void context::make_current(native_window_t window)
{     
    // glXMakeContextCurrent doesn't like None for surface.  (mesa 9.2)
    // so instead of None we use the temporary window surface
    glXMakeCurrent(pimpl_->disp, pimpl_->temp_surface, pimpl_->context);

    pimpl_->release_surface();

    if (window == wdk::NULL_WINDOW)
        return;

    GLXWindow surface = glXCreateWindow(pimpl_->disp, pimpl_->config, window, NULL);

    if (!glXMakeCurrent(pimpl_->disp, surface, pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = surface;
    pimpl_->surface_type = drawable_type::window;
}

void context::make_current(native_pixmap_t pixmap)
{
    // glXMakeContextCurrent doesn't like None for surface.  (mesa 9.2)
    // so instead of None we use the temporary window surface
    glXMakeCurrent(pimpl_->disp, pimpl_->temp_surface, pimpl_->context);

    pimpl_->release_surface();

    if (pixmap == wdk::NULL_PIXMAP)
        return;

    GLXPixmap surface = glXCreatePixmap(pimpl_->disp, pimpl_->config, pixmap, NULL);

    if (!glXMakeCurrent(pimpl_->disp, surface, pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = surface;
    pimpl_->surface_type = drawable_type::pixmap;    
}

void context::swap_buffers()
{
    assert(pimpl_->surface && "context has no valid surface. did you forget to call make_current?");

    glXSwapBuffers(pimpl_->disp, pimpl_->surface);
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

} // wdk
