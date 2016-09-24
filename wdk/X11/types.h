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

// the X headers are ugly and pollute the global space badly with macros. (such as Status)
// we do *not* want to include those.

//#include <X11/Xlib.h>
//#include <X11/extensions/Xrandr.h>

#include <memory>

struct _XDisplay;
union  _XEvent;

namespace wdk
{
    namespace detail {

        typedef unsigned long XID;
        typedef XID Window;
        typedef XID Pixmap;

    } // detail

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

    typedef xid_t<detail::Window, 0> native_window_t;
    typedef xid_t<detail::Pixmap, 1> native_pixmap_t;
    typedef int              native_handle_t;
    typedef _XDisplay*       native_display_t;

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

        native_event_t();
        native_event_t(const _XEvent& e);

        operator const _XEvent& () const;

        native_window_t get_window_handle() const;

        const _XEvent& get() const;

        type identity() const;

    private:
        std::shared_ptr<_XEvent> event_;

    };

    const native_window_t  NULL_WINDOW  {0};

} // wdk
