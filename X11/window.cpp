

// on ubuntu the package for this header is libxxf86vm-dev (double X on purpose!)
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/Xutil.h>
#include <cassert>
#include <stdexcept>
#include <cstring>
#include "../events.h"
#include "../event.h"
#include "../window.h"
#include "error_handler.h"

namespace {
    struct hint {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long          inputMode;
        unsigned long status;
    };
} // namespace

namespace wdk
{

struct window::impl {
    Window   window;
    window_param param;
    Atom atom_delete_window;
    Display* disp;
};

window::window(native_display_t disp)
{
    pimpl_.reset(new impl);
    window_param empty = {};
    pimpl_->param              = empty;
    pimpl_->atom_delete_window = XInternAtom(disp, "WM_DELETE_WINDOW", False);
    pimpl_->window             = 0L;
    pimpl_->disp               = disp;
}

window::~window()
{
    if (exists())
        close();
}

void window::create(const window_param& how)
{
    assert(!exists());

    const native_display_t display_handle = pimpl_->disp;
    const int screen                      = DefaultScreen(display_handle);
    const Window root                     = RootWindow(display_handle, screen);

    XVisualInfo vistemplate = {};
    vistemplate.visualid    = how.visualid ? how.visualid : 0;
    const long visual_mask  = how.visualid ? VisualIDMask : 0;

    int num_visuals = 0;
    XVisualInfo* visinfo = XGetVisualInfo(display_handle, visual_mask, &vistemplate, &num_visuals);
    if (!visinfo || !num_visuals)
        throw std::runtime_error("cannot get visual info");

    XSetWindowAttributes attr = {};
    attr.background_pixel     = 0;
    attr.border_pixel         = 0;
    attr.colormap             = XCreateColormap(display_handle, root, visinfo->visual, AllocNone);
    attr.event_mask           = SubstructureNotifyMask | StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | FocusChangeMask;

    uint_t window_width     = how.width;
    uint_t window_height    = how.height;
    bitflag_t window_props  = how.props;
    unsigned long attr_mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    if (how.fullscreen)
    {
        // find the current display mode
        XF86VidModeModeLine mode = {0};
        int dotclock = 0;
        if (XF86VidModeGetModeLine(display_handle, screen, &dotclock, &mode) == False)
            throw std::runtime_error("failed to get current fullscreen mode");

        // adjust window w/h to fullscreen
        window_width  = mode.hdisplay;
        window_height = mode.vdisplay;
        window_props  = 0; // no border or resize is possible

        // add a flag to grab events that would otherwise go to the window manager
        attr_mask |= CWOverrideRedirect; 
        attr.override_redirect = True;
    }

    const uint_t chosen_visual = visinfo->visualid;

    factory<Window> win_factory(display_handle);

    Window win = win_factory.create([&](Display* dpy) 
    {
        Window ret = XCreateWindow(dpy, 
            root, 
            0, 0, 
            window_width, 
            window_height, 
            0,
            visinfo->depth, 
            InputOutput,
            visinfo->visual,
            attr_mask,
            &attr);
        return ret;
    });

    XFree(visinfo);
    
    if (win_factory.has_error())
        throw std::runtime_error("failed to create window");

    XSetWMProtocols(display_handle, win, &pimpl_->atom_delete_window, 1);
    XMapWindow(display_handle, win);

    if (how.fullscreen)
    {
        XWarpPointer(display_handle, None, win, 0, 0, 0, 0, 0, 0);
        XGrabKeyboard(display_handle, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        XGrabPointer(display_handle, win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
    }
    else
    {
        if (!(how.props & wdk::WP_BORDER))
        {
            // get rid of window decorations
            hint hints = {0};
            hints.flags = 2;         // window decorations flag
            hints.decorations = 0;   // ...say bye bye
            Atom property = XInternAtom(display_handle, "_MOTIF_WM_HINTS", True);
            XChangeProperty(display_handle, win, property, property, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&hints), 5);
        }
        if (!(how.props & wdk::WP_RESIZE))
        {
            // make window unresizeable
            XSizeHints* hints = XAllocSizeHints();
            hints->flags      = PMinSize | PMaxSize;
            hints->min_width  = hints->max_width  = how.width;
            hints->min_height = hints->max_height = how.height;
            XSetWMSizeHints(display_handle, win, hints, XInternAtom(display_handle, "WM_SIZE_HINTS", True));
            XSetWMNormalHints(display_handle, win, hints);
            XFree(hints);
        }
        if (!how.title.empty())
            XSetStandardProperties(display_handle, win, how.title.c_str(), how.title.c_str(), None, (char**)NULL, 0, NULL);
    }

    Window dummy = 0;
    int x, y;
    unsigned int width, height = 0;
    unsigned int border = 0;
    unsigned int depth  = 0;
    XGetGeometry(display_handle, win, &dummy, &x, &y, &width, &height, &border, &depth);

    //printf("%d, %d\n\n", x, y);

    assert(window_width  == width);
    assert(window_height == height);

    pimpl_->window = win;
    pimpl_->param  = how;
    pimpl_->param.width    = (uint_t)width;
    pimpl_->param.height   = (uint_t)height;
    pimpl_->param.visualid = chosen_visual;
    pimpl_->param.props    = window_props;
    pimpl_->disp           = display_handle;


    XEvent send = {0};
    send.type = CreateNotify;
    send.xcreatewindow.window = win;
    send.xcreatewindow.y = y;
    send.xcreatewindow.x = x;
    send.xcreatewindow.width = width;
    send.xcreatewindow.height = height;
    XSendEvent(display_handle, win, False, 0, &send);
}


void window::close()
{
    assert(exists());

    Display* disp = pimpl_->disp;
    Window window = pimpl_->window;

    XEvent send = {0};
    send.type   = DestroyNotify;
    send.xdestroywindow.window = pimpl_->window;
    XSendEvent(disp, pimpl_->window, False, 0, &send);

    if (pimpl_->param.fullscreen)
    {
        XUngrabKeyboard(disp, CurrentTime);
        XUngrabPointer(disp, CurrentTime);
    }
    XUnmapWindow(disp, window);
    XDestroyWindow(disp, window);

    window_param empty = {};
    pimpl_->window = 0L;
    pimpl_->param  = empty;
}

uint_t window::width() const
{
    Window dummy;
    int x, y;
    unsigned int width, height;
    unsigned int border;
    unsigned int depth;
    XGetGeometry(pimpl_->disp, pimpl_->window, &dummy, &x, &y, &width, &height, &border, &depth);
    return (uint_t)width + 2 * border;
}

uint_t window::height() const
{
    assert(!"unimplemented function");

    // todo: how to get the title bar height, have to query the window manager?
    return 0;
}

uint_t window::visualid() const
{
    return pimpl_->param.visualid;
}

uint_t window::surface_width() const
{
    return pimpl_->param.width;
}

uint_t window::surface_height() const
{
    return pimpl_->param.height;
}

bool window::exists() const
{
    return (pimpl_->window != 0L);
}

bool window::dispatch_event(const event& ev) 
{
    // if the event is not for this window, return quickly
    if (ev.window.xid != pimpl_->window)
        return false;

    const XEvent* event = &ev.ev;

    const native_window_t win   = {pimpl_->window};
    const native_surface_t surf = {pimpl_->window};
    const native_display_t disp = {pimpl_->disp};

    switch (ev.type)
    {
        case event_type::window_gain_focus:
        {
            if (event_gain_focus)
                event_gain_focus(window_event_focus{win, disp});
        }
        break;
            
        case event_type::window_lost_focus:
        {
            if (event_lost_focus)
                event_lost_focus(window_event_focus{win, disp});
        }
        break;
        
        case event_type::window_paint:
        {
            if (event_paint)
            {
                event_paint(window_event_paint{event->xexpose.x, 
                    event->xexpose.y,
                    event->xexpose.width,
                    event->xexpose.height,
                    win, surf, disp});
            }
        }
        break;

        // resize, move, map/unmap, border size change
        case event_type::window_configure:
        {
            if (event->xconfigure.width != (int)pimpl_->param.width || 
                event->xconfigure.height != (int)pimpl_->param.height)
            {
                if (event_resize) {
                    event_resize(window_event_resize{event->xconfigure.width,
                        event->xconfigure.height,
                        win, surf, disp});
                }
                // the width and height members are set to the inside size of the 
                // window not including the border. border_width is the width of the window border (in pixels)
                pimpl_->param.width  = event->xconfigure.width;
                pimpl_->param.height = event->xconfigure.height;
            }
        }
        break;

        case event_type::window_close:
        {
            if ((Atom)event->xclient.data.l[0] == pimpl_->atom_delete_window && event_close)
            {
                window_event_close e = {false, win, disp};
                e.should_close = false;
                event_close(e);
                if (e.should_close)
                    close();
            }
        }
        break;
    
        // // the X server can report CreateNotify and DestroyNotify events when client applications
        // // create and destroy windows. in order to receive these events we need to specificy
        // // SubstructureNotifyMask in the event mask.
        // // These events notify us of any child windows being created.
        case event_type::window_create:
        {
            if (event_create)
            {
                event_create(window_event_create{event->xcreatewindow.x, 
                    event->xcreatewindow.y, 
                    event->xcreatewindow.width,
                    event->xcreatewindow.height,
                    pimpl_->param.fullscreen, 
                    win, surf, disp});
            }
        }
        break;

        case event_type::window_destroy:
        {
            if (event_destroy)
                event_destroy(window_event_destroy{win, disp});
        }
        break;

        default:
           return false;
    }
    return true;
}

native_window_t window::handle() const
{
    if (!pimpl_->window)
        return wdk::NULL_WINDOW;

    return native_window_t {pimpl_->window};
}

native_surface_t window::surface() const
{
    if (!pimpl_->window)
        return wdk::NULL_SURFACE;

    return native_surface_t {pimpl_->window};
}

native_display_t window::display() const
{
    if (!pimpl_->window)
        return (native_display_t)wdk::NULL_HANDLE;
    return pimpl_->disp;
}

} // wdk
