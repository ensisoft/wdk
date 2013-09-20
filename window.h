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

#include <functional>   // for function, bind
#include <memory>       // for unique_ptr
#include <string>       
#include "types.h"
#include "utility.h"

namespace wdk
{
    struct event;   

    // window events
    struct window_event_create;
    struct window_event_paint;
    struct window_event_resize;
    struct window_event_focus;
    struct window_event_query_close;
    struct window_event_destroy;

    class display;

    // create a window for showing/rendering content 
    class window : noncopyable
    {
    public:
        enum properties {
            HAS_BORDER   = 0x1,
            CAN_RESIZE   = 0x2,
            CAN_MOVE     = 0x4
        };

        std::function<void (const window_event_create&)>   event_create;
        std::function<void (const window_event_paint&)>    event_paint;
        std::function<void (const window_event_resize&)>   event_resize;
        std::function<void (const window_event_focus&)>    event_lost_focus;
        std::function<void (const window_event_focus&)>    event_gain_focus;
        std::function<void (window_event_query_close&)>    event_query_close;
        std::function<void (const window_event_destroy&)>  event_destroy;

        window(const display& disp);
       ~window();

        // window parameters defines how the window is to be created.
        // if fullscreen is true then window properties are ignored
        // and window will be created without border, unresizeable and 
        // dimensions will be current video mode dimensions.
        struct params {
            std::string title;      // optional window title to be display in the titlebar (if any)
            uint_t      width;      // renderable surface width (inside client area of the window)
            uint_t      height;     // renderable surface height (inside client area of the window)
            uint_t      visualid;   // GL color buffer id for GL rendering compatibility
            bitflag_t   props;      // bitflags of window properties
            bool        fullscreen; // fullscreen flag. if true title and props are ignored.
        };

        // create the window.
        void create(const params& how);

        // close the window. 
        void close();
        
        // get current window width in screen units. 
        uint_t width() const;

        // get current window height in screen  units
        uint_t height() const;
        
        // get the current window visual id. 
        uint_t visualid() const;

        // get drawable surface width in px
        uint_t surface_width() const;
        
        // get drawable surface height in px
        uint_t surface_height() const;

        // returns true if window exists otherwise false
        bool exists() const;

        // dispatch the given event. returns true
        // if message was dispatched, otherwise false
        bool dispatch_event(const event& ev);

        // get native window handle
        native_window_t handle() const;

        // get the display handle where the window is created
        native_display_t display() const;
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

    // interface for listening to window events
    class window_listener
    {
    public:
        virtual ~window_listener() {}
        virtual void on_create(const window_event_create&) {}
        virtual void on_paint(const window_event_paint&) {}
        virtual void on_resize(const window_event_resize&) {}
        virtual void on_lost_focus(const window_event_focus&) {}
        virtual void on_gain_focus(const window_event_focus&) {}
        virtual void on_query_close(window_event_query_close&) {}
        virtual void on_destroy(const window_event_destroy&) {}

    };

    // connect all events in the given window to the given window listener
    inline void connect(window& win, window_listener* listener)
    {
        namespace args = std::placeholders;
        win.event_create      = std::bind(&window_listener::on_create, listener, args::_1);
        win.event_paint       = std::bind(&window_listener::on_paint, listener, args::_1);
        win.event_resize      = std::bind(&window_listener::on_resize, listener, args::_1);
        win.event_lost_focus  = std::bind(&window_listener::on_lost_focus, listener, args::_1);
        win.event_gain_focus  = std::bind(&window_listener::on_gain_focus, listener, args::_1);
        win.event_query_close = std::bind(&window_listener::on_query_close, listener, args::_1);
        win.event_destroy     = std::bind(&window_listener::on_destroy, listener, args::_1);
    }

} // wdk
