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

#include <string>

#ifdef _MSC_VER
#  undef hyper
#endif

// X11 poison 
#undef None

namespace wdk
{

    // available mouse buttons. a "standard" mouse is expected to have 
    // left and right buttons and a mouse wheel. 
    // some mice also have more buttons in "non-standard" locations
    // these are named thumbN buttons here.
    enum class MouseButton {
        None, 
        // Left right buttons
        Left, Right, 
        // Wheel button, wheel scroll up / down
        Wheel, WheelScrollUp, WheelScrollDown, 
        // Possible different thumb buttons
        Thumb1, Thumb2, Thumb3, Thumb4
    };

    // keyboard event processing has 3 potential levels of operation available to applications.
    // going from lowest to higher the operations are as follows:
    // 
    // 1) scan/key codes: each key event carries a native keycode information and correspond
    // directly to the physical keys pressed on the keyboard. Application could use an API
    // to map virtual keys to native keycodes based on the current keyboard driver and hence
    // derive keycode values for known keys available on the keyboard.
    // 
    // 2) virtual keysyms/modifier keys: the scan/key codes are translated and mapped to virtual
    // keysym and keymod pairs. keysym defines the key that was pressed and keymod defines the
    // modifier keys that were also depressed at the time when event was generated.
    // 
    // 3) completely translated character input. Series of keypresses are translated to
    // Unicode characters for cooked character based input.

    // possible modifiers, used as a bitwise OR flag
    enum class Keymod {
        None, Shift, Control, Alt
    };

    // primitive key symbols. this list represents a physical keys available
    // on most keyboards. the meaning of each raw key depends on the application.
    // key translation to characters is keyboard layout specific. Some keyboard
    // layouts have different mappings for different characters. For example with US 
    // layout double quote (") is a single key and to generate the same character with 
    // FI layout user needs to press Shift+2.
    // therefore this list does not contain any keys that are layout specific but maps
    // directly to physical keys on the keyboard
    enum class Keysym 
    {
        None,                   // no known key
        Backspace,
        Tab,
        Enter,
        Space,
        Key0, 
        Key1, 
        Key2, 
        Key3, 
        Key4, 
        Key5, 
        Key6,
        Key7, 
        Key8, 
        Key9,
        KeyA, 
        KeyB, 
        KeyC, 
        KeyD, 
        KeyE, 
        KeyF, 
        KeyG, 
        KeyH, 
        KeyI, 
        KeyJ, 
        KeyK, 
        KeyL, 
        KeyM, 
        KeyN, 
        KeyO, 
        KeyP, 
        KeyQ, 
        KeyR, 
        KeyS, 
        KeyT, 
        KeyU, 
        KeyV, 
        KeyW, 
        KeyX, 
        KeyY,
        KeyZ,
        F1, 
        F2, 
        F3, 
        F4, 
        F5, 
        F6, 
        F7, 
        F8, 
        F9,
        F10, 
        F11, 
        F12,
        ControlR,
        ControlL,
        ShiftR,
        ShiftL,
        AltL,
        CapsLock,
        Insert,
        Del,
        Home,
        End,
        PageUp,
        PageDown,
        ArrowLeft,
        ArrowUp,
        ArrowDown,
        ArrowRight,
        Escape
    }; 

    // Map key modifier to a human readable name.
    std::string ToString(Keymod mod);

    // Map key symbol to a human readable name.
    std::string ToString(Keysym sym);

    // Map mouse button to a human readable name.
    std::string ToString(MouseButton btn);

} // wdk


