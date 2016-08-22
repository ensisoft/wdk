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

#include <functional> // for funtion
#include <memory>  // for unique_ptr
#include <utility> // for pair
#include <string>
#include "utility.h"
#include "types.h"

namespace wdk
{
    struct window_event_create;
    struct window_event_paint;
    struct window_event_resize;
    struct window_event_focus;
    struct window_event_want_close;
    struct window_event_keyup;
    struct window_event_keydown;    
    struct window_event_char;
    struct window_event_mouse_move;
    struct window_event_mouse_press;
    struct window_event_mouse_release;

    class window : noncopyable
    {
    public:
        // character encoding for char events. defaults to utf8
        enum class encoding {
            ascii, ucs2, utf8
        };
        
        // event callbacks
        std::function<void (const window_event_create&)>     on_create;
        std::function<void (const window_event_paint&)>      on_paint;
        std::function<void (const window_event_resize&)>     on_resize;
        std::function<void (const window_event_focus&)>      on_lost_focus;
        std::function<void (const window_event_focus&)>      on_gain_focus;
        std::function<void (const window_event_want_close&)> on_want_close;
        std::function<void (const window_event_keydown&)>    on_keydown;
        std::function<void (const window_event_keyup&)>      on_keyup;
        std::function<void (const window_event_char&)>       on_char;
        std::function<void (const window_event_mouse_move&)> on_mouse_move;
        std::function<void (const window_event_mouse_press&)> on_mouse_press;
        std::function<void (const window_event_mouse_release&)> on_mouse_release;

        window();

       ~window();

        // create the window with the given dimension and flags.
        // window must not exist before.
        //
        // If you're planning on using this window for OpenGL drawing
        // you should pass in a visual id that identifies your OpenGL ćonfiguration.
        // If visualid is 0 the window may not be compatible with your opengl config.
        void create(const std::string& title, uint_t width, uint_t height, uint_t visualid, 
            bool can_resize = true, bool has_border = true, bool initially_visible = true);

        // hide the window if currently visible. (shown)
        void hide();

        // show the window if currently hidden. 
        void show();

        // destroy the window. window must have been created before.
        void destroy();

        // move window to x,y position with respect to it's parent. (desktop)
        // precondition: not fullscreen
        // precondition: window has been created
        void move(int x, int y);

        // toggle between fullscreen/windowed mode.
        void set_fullscreen(bool fullscreen);

        // set input focus to this window
        void set_focus();

        // set new drawable surface size
        void set_size(uint_t width, uint_t height);

        // set new character encoding for character events
        void set_encoding(encoding e);

        // check and process one event if available. if event is not for this window it's discarded.
        void poll_one_event();

        // wait and process one event. if event is not for this window it's discarded
        void wait_one_event();

        // get and process. events that are not for this window are simply discarded.
        void process_all_events();

        // process the given event. the event should be for this window.
        void process_event(const native_event_t& ev);

        // you don't really want to use this under normal operations. only when you must
        // make sure that the cached state in Xlib reflects the state we're trying set.
        void sync_all_events();

        // get the current drawable window surface height
        uint_t surface_height() const;

        // get the current drawable window surface width
        uint_t surface_width() const;

        // returns true if window currently exists. otherwise false.
        bool exists() const;

        bool is_fullscreen() const;

        // get the current character encoding. the default is utf8
        encoding get_encoding() const;

        // get native window handle
        native_window_t handle() const;

        std::pair<uint_t, uint_t> min_size() const;
        std::pair<uint_t, uint_t> max_size() const;
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

} // wdk
