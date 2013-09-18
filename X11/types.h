
#pragma once

#include <X11/Xlib.h>

namespace wdk
{
    typedef int      native_handle_t;
    typedef XEvent   native_event_t;
    typedef Display* native_display_t;
    typedef int      native_vmode_t;

    // wrapper structure to make XID objects separate handle types
    // so we can add more type safety and overload
    template<typename T, int discriminator>
    struct xid_t {
        T xid;

        operator T () const
        {
            return xid;
        }
    };

    template<typename T, int discriminator> inline
    bool operator==(const xid_t<T, discriminator>& rhs, const xid_t<T, discriminator>& lhs)
    {
        return rhs.xid == lhs.xid;
    }

    template<typename T, int discriminator> inline
    bool operator!=(const xid_t<T, discriminator>& rhs, const xid_t<T, discriminator>& lhs)
    {
        return rhs.xid != lhs.xid;
    }

    typedef xid_t<Window, 0> native_window_t;
    typedef xid_t<Drawable, 1> native_surface_t;
    typedef xid_t<Pixmap, 2> native_pixmap_t;

    enum {
        NULL_HANDLE        = 0,
        DEFAULT_VIDEO_MODE = 0
    };    

    const native_window_t NULL_WINDOW {0};
    const native_surface_t NULL_SURFACE {0};
    const native_pixmap_t NULL_PIXMAP {0};

} // wdk
