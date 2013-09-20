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

#include "keys.h"
#include "types.h"

namespace wdk
{
    // these are the event objects that are passed as parameters to the
    // event dispatching routines. these events are represented
    // in temporal order. some of them occur only once during a lifetime
    // of a window while others might occur several times.
    // mouse and keyboard events are low level events and thus limited 
    // use for the client to use directly. rather they need to be passed
    // to the mouse/keyboard objects for processing.

    // on_create, happens on succesful window create() 
    struct window_event_create {
        int x, y;                       // x, y location on the screen (top left corner)
        int width;                   // surface width
        int height;                  // surface height
        bool fullscreen;                // fullscreen or not
        native_window_t  window;         
        native_display_t display;
    };

    // on_paint, x, y is the top left corner of the surface area
    // relative to to the top left origin. w and h are the width and height
    // of the dirty rectangle respectively.
    struct window_event_paint {
        int x, y;                       // x, y origin of the dirty rectangle
        int width;                   // width of the dirty rect
        int height;                  // height of the dirty rect
        native_window_t  window;        
        native_display_t display;       
    };

    // on_resize, new width and height of the window surface area.
    struct window_event_resize {
        int width;                  // surface width
        int height;                 // surface height
        native_window_t window;         
        native_display_t display;       
    };

    // on_gain_focus, on_lost_focus
    struct window_event_focus {
        native_window_t window;
        native_display_t display;
    };

    // on_query_close
    struct window_event_query_close {
        bool should_close;
        native_window_t window;
        native_display_t display;
    };

    // on_close, window was closed either by user or
    // by calling close()
    struct window_event_destroy {
        native_window_t window;
        native_display_t display;
    };


    struct keyboard_event_keypress {
        keysym symbol;
        keymod modifiers;
        native_window_t window;
    };

    // ucs2 unicode character
    struct keyboard_event_char {
        long value;    
        native_window_t window;
    };

    typedef keyboard_event_keypress keyboard_event_keydown;
    typedef keyboard_event_keypress keyboard_event_keyup;

} // wdk
