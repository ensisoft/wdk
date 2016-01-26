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

#pragma once

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "atoms.h"

namespace wdk
{
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
    typedef xid_t<Pixmap, 1> native_pixmap_t;
    typedef int              native_handle_t;
    typedef Display*         native_display_t;
    typedef Display*         egl_display_t;
    typedef native_window_t  egl_handle_t;

    class native_event_t 
    {
    public:
        enum class type {
            window_gain_focus,
            window_lost_focus,
            window_resize,
            window_create,
            window_destroy,
            window_keydown,
            window_keyup,
            window_char,
            window_mouse_move,
            window_mouse_press,
            window_mouse_release,

            system_resolution_change,
            other
        };

        native_event_t() 
        {
            event_ = XEvent{0};
        }
        native_event_t(const XEvent& e) : event_(e)
        {}
        operator const XEvent& () const
        {
            return event_;
        }
        native_window_t get_window_handle() const
        {
            if  (event_.type == KeymapNotify)
                return native_window_t { 0 };
            else if (event_.type == CreateNotify)
                return native_window_t { event_.xcreatewindow.window };
            else if (event_.type == MapNotify)
                return native_window_t { event_.xmap.window };

            return native_window_t  {event_.xany.window};
        }
        const XEvent& get() const
        {
            return event_;
        }
        
        type identity() const
        {
            if ((event_.type - XRandREventBase) == RRScreenChangeNotify)
                return type::system_resolution_change;

            switch (event_.type)
            {
                case FocusIn:         return type::window_gain_focus;
                case FocusOut:        return type::window_lost_focus;
                case ConfigureNotify: return type::window_resize;
                case CreateNotify:    return type::window_create;
                case DestroyNotify:   return type::window_destroy;
                case KeyPress:        return type::window_keydown;
                case KeyRelease:      return type::window_keyup;
                case MotionNotify:    return type::window_mouse_move;
                case ButtonPress:     return type::window_mouse_press;
                case ButtonRelease:   return type::window_mouse_release;
                case MapNotify:
                    if (event_.xany.send_event)
                        return type::window_char;

                default:
                    break;
            }
            return type::other;
        }
    private:
        XEvent event_;        
    };

    const native_window_t  NULL_WINDOW  {0};

} // wdk
