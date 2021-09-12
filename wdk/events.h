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
#include <string>

#include "wdk/keys.h"
#include "wdk/bitflag.h"

namespace wdk
{
    // These are the event objects that are passed as parameters to the
    // event dispatching routines. these events are listed here in a rough
    // in temporal order. Some of them occur only once during a lifetime
    // of a window while others might occur several times.
    // Mouse and keyboard events are low level events and thus of limited
    // use for the client to use directly. Rather they need to be translated
    // and mapped to higher level symbolic key and mouse presses.

    // Window was created and is now visible on the screen
    struct WindowEventCreate {
        int x = 0;
        int y = 0;                          // x, y location on the screen (top left corner)
        int width  = 0;                     // surface width
        int height = 0;                     // surface height
    };

    // Window has a dirty rectangle and needs to be repainted.
    // When this message is sent depends on the implementation
    // and its understanding of when the contents of the window
    // have become invalid and need repainting.
    struct WindowEventPaint {
        int x = 0;
        int y = 0;                      // x, y origin of the dirty rectangle (top left corner within the window)
        int width  = 0;                 // width of the dirty rect
        int height = 0;                 // height of the dirty rect
    };

    // Window has been resized.
    struct WindowEventResize {
        int width  = 0;                     // new surface width
        int height = 0;                     // new surface height
    };

    // Window has gained or lost input focus.
    struct WindowEventGainFocus {};
    struct WindowEventLostFocus {};

    // Window close has been requested.
    struct WindowEventWantClose {};

    // Window was destroyed.
    struct WindowEventDestroy {};

    // Keyboard was pressed when the window had input focus.
    // This message indicates the release of the key.
    struct WindowEventKeyUp {
        Keysym symbol = Keysym::None;
        bitflag<Keymod> modifiers;
    };

    // Keyboard was pressed when the window had input focus.
    // This messages indicates the press of the key.
    struct WindowEventKeyDown {
        Keysym symbol = Keysym::None;
        bitflag<Keymod> modifiers;
    };

    // Series of key pressed were translated into a 'character'. 
    // The encoding used for the character depends on the Encoding
    // setting of the window.
    struct WindowEventChar {
        union {
            uint8_t  ascii;
            uint16_t ucs2;
            uint32_t ucs4;
            uint8_t  utf8[4];
        };
    };

    // todo: support multiple simultaneous mouse buttons.
    // todo: support double clicks

    // Mouse was moved within the window area when the window 
    // had input focus.
    struct WindowEventMouseMove {
        // x,y coordinate of the mouse pointer in window coordinates.        
        int window_x = 0;
        int window_y = 0;   

        // x, y coordinate of the mouse pointer in desktop coordinates.
        int global_x = 0;
        int global_y = 0;  
        
        // pressed mouse button if any.
        MouseButton btn = MouseButton::None;   
        // Keyboard modifiers if any.
        bitflag<Keymod> modifiers;
    };

    // A mouse button was clicked in the window area.
    // This message indicates the beginning (press) of the button click.
    struct WindowEventMousePress {
        // x,y coordinate of the mouse pointer in window coordinates.                
        int window_x = 0;
        int window_y = 0;

        // x, y coordinate of the mouse pointer in desktop coordinates.
        int global_x = 0;
        int global_y = 0;

        // pressed mouse button if any.
        MouseButton btn = MouseButton::None;
        // keyboard modifiers if any.
        bitflag<Keymod> modifiers;
    };

    // A mouse button was clicked in the window area. 
    // This message indicates the end (release) of the button click.
    struct WindowEventMouseRelease {
        // x,y coordinate of the mouse pointer in window coordinates.                        
        int window_x = 0;
        int window_y = 0;

        // x, y coordinate of the mouse pointer in desktop coordinates.        
        int global_x = 0;
        int global_y = 0;

        // pressed mouse button if any.
        MouseButton btn = MouseButton::None;
        // keyboard modifiers if any.
        bitflag<Keymod> modifiers;
    };


    // This event indicates that the system's video mode/resolution
    // was changed.
    struct SystemEventResolutionChange {
        // The new horizontal resolution.
        unsigned xres = 0;
        // The new vertical resolution.
        unsigned yres = 0;
    };

    template<typename T>
    struct EventTraits;

    template<>
    struct EventTraits<WindowEventCreate> {
        static constexpr auto name = "create";
    };
    template<>
    struct EventTraits<WindowEventPaint> {
        static constexpr auto name = "paint";
    };
    template<>
    struct EventTraits<WindowEventResize> {
        static constexpr auto name = "resize";
    };
    template<>
    struct EventTraits<WindowEventGainFocus> {
        static constexpr auto name = "gain_focus";
    };
    template<>
    struct EventTraits<WindowEventLostFocus> {
        static constexpr auto name = "lost_focus";
    };
    template<>
    struct EventTraits<WindowEventWantClose> {
        static constexpr auto name = "want_close";
    };
    template<>
    struct EventTraits<WindowEventDestroy> {
        static constexpr auto name = "destroy";
    };
    template<>
    struct EventTraits<WindowEventKeyUp> {
        static constexpr auto name = "key_up";
    };
    template<>
    struct EventTraits<WindowEventKeyDown> {
        static constexpr auto name = "key_down";
    };
    template<>
    struct EventTraits<WindowEventChar> {
        static constexpr auto name = "character";
    };
    template<>
    struct EventTraits<WindowEventMouseMove> {
        static constexpr auto name = "mouse_move";
    };
    template<>
    struct EventTraits<WindowEventMousePress> {
        static constexpr auto name = "mouse_press";
    };
    template<>
    struct EventTraits<WindowEventMouseRelease> {
        static constexpr auto name = "mouse_release";
    };
    template<>
    struct EventTraits<SystemEventResolutionChange> {
        static constexpr auto name = "system_resolution_change";
    };

    template<typename Event>
    const char* GetEventName(const Event& event)
    { return EventTraits<Event>::name; }

} // wdk
