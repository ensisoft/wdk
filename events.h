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

    // window was created and is visible on the screen
    struct window_event_create {
        int x, y;                       // x, y location on the screen (top left corner)
        int width;                      // surface width
        int height;                     // surface height
    };

    // window has a dirty rectangle and needs to be repainted.
    struct window_event_paint {
        int x, y;                       // x, y origin of the dirty rectangle (top left corner within the window)
        int width;                      // width of the dirty rect
        int height;                     // height of the dirty rect
    };

    // window has been resized
    struct window_event_resize {
        int width;                      // new surface width
        int height;                     // new surface height
    };

    // do not use directly, see the typedefs below
    struct window_event_generic {};

    // do not use directly, see the typedefs below
    struct window_event_keypress {
        keysym symbol;
        keymod modifiers;
    };

    // input character. the meaning of value depends
    // on the ime setting used to translate input
    struct window_event_char {
        union {
            uint8_t  ascii;
            uint16_t ucs2;
            uint32_t ucs4;
            uint8_t  utf8[4];
        };
    };

    // window lost/gained focus
    typedef window_event_generic window_event_focus;

    // user wants to close the window, if this is desired call close on the window to respond.
    typedef window_event_generic window_event_want_close;

    // window was destroyed and has been removed from the scren.
    // note that the handle is no longer a valid window handle!
    typedef window_event_generic window_event_destroy;

    // on keyboard key press
    typedef window_event_keypress window_event_keydown;

    // on keyboard key up
    typedef window_event_keypress window_event_keyup;

} // wdk
