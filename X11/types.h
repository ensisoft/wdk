
#pragma once

#include <X11/Xlib.h>

namespace wdk
{
    typedef Window   native_window_t;
    typedef int      native_handle_t;
    typedef XEvent   native_event_t;
    typedef Display* native_display_t;
    typedef Drawable native_surface_t;
    typedef int      native_vmode_t;

    enum {
        NULL_SURFACE       = 0,
        NULL_WINDOW        = 0,
        NULL_HANDLE        = 0,
        DEFAULT_VIDEO_MODE = 0
    };    

} // wdk
