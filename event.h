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

namespace wdk
{
    enum class event_type {
    	none,
    	window_create,
    	window_paint,
    	window_configure,
    	window_gain_focus,
    	window_lost_focus,
    	window_close,
    	window_destroy,
    	keyboard_keyup,
    	keyboard_keydown,
    	keyboard_char,
    	mouse_move
    };

    // native window system event
    struct event {
        native_window_t window;
        native_event_t  ev;
        event_type type;
    };
	
    inline
    bool is_window_event(event_type e)
    {
        return (e >= event_type::window_create && e <= event_type::window_close);
    }

    inline
    bool is_keyboard_event(event_type e)
    {
        return (e >= event_type::keyboard_keyup && e <= event_type::keyboard_char);
    }

} // wdk