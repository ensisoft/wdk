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

#include <cstdint>
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

    struct window_event_generic {
        native_window_t window;
        native_display_t display;
    };

    struct keyboard_event_keypress {
        keysym symbol;
        keymod modifiers;
        native_window_t window;
    };

    // input character. the meaning of value depends
    // on the ime setting used to translate input
    struct ime_event_char {
        union {
            uint8_t  ascii;
            uint16_t ucs2;
            uint32_t ucs4;
            uint8_t  utf8[4];
        };
        native_window_t window;
    };


#ifndef HAS_EVENT_TYPEDEFS
    // on_gain_focus, on_lost_focus
    typedef window_event_generic window_event_focus;

    // user wants to close the window, if this is desired
    // call close on the window
    typedef window_event_generic window_event_query_close;

    // window was destroyed as a result to calling close
    typedef window_event_generic window_event_destroy;

    // on keyboard key press
    typedef keyboard_event_keypress keyboard_event_keydown;

    // on keyboard key up
    typedef keyboard_event_keypress keyboard_event_keyup;

    #define HAS_EVENT_TYPEDEFS
#endif

} // wdk
