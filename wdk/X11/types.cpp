// Copyright (c) 2016 Sami Väisänen, Ensisoft
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


#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "wdk/X11/atoms.h"
#include "wdk/X11/types.h"

namespace wdk
{

static_assert(sizeof(detail::Window) == sizeof(Window),
    "X11 window type is not unsigned long");
static_assert(sizeof(detail::Pixmap) == sizeof(Pixmap),
    "X11 pixmap type is not unsigned long");

native_event_t::native_event_t() : event_(std::make_shared<XEvent>())
{}

native_event_t::native_event_t(const XEvent& e) : event_(std::make_shared<XEvent>(e))
{}

native_event_t::operator const XEvent& () const
{
    return *event_;
}

native_window_t native_event_t::get_window_handle() const
{
    if  (event_->type == KeymapNotify)
        return native_window_t { 0 };
    else if (event_->type == CreateNotify)
        return native_window_t { event_->xcreatewindow.window };
    else if (event_->type == MapNotify)
        return native_window_t { event_->xmap.window };

    return native_window_t  {event_->xany.window};
}

const XEvent& native_event_t::get() const
{
    return *event_;
}

native_event_t::type native_event_t::identity() const
{
    if ((event_->type - XRandREventBase) == RRScreenChangeNotify)
        return type::system_resolution_change;

    switch (event_->type)
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
            if (event_->xany.send_event)
                return type::window_char;

        default:
            break;
    }
    return type::other;
}


} // wdk