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
