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

#include "types.h"

#ifdef _MSC_VER
#  undef hyper
#endif


namespace wdk
{
    // keyboard event processing has 3 potential levels of operation available to applications.
    // going from lowest to higher the operations are as follows:
    // 
    // 1) scan/key codes: each key event carries a native keycode information and correspond
    // directly to the physical keys pressed on the keyboard. Application could use an API
    // to map virtual keys to native keycodes based on the current keyboard driver and hence
    // derive keycode values for known keys available on the keyboard.
    // 
    // 2) virtual keysyms/modifier keys: the scan/key codes are translated and mapped to virtual
    // keysym and keymod airs. keysym defines the key that was pressed and keymod defines the
    // modifier keys that were also depressed at the time when event was generated.
    // 
    // 3) completely translated character input. Series of keypresses are translated to
    // Unicode characters for cooked character based input.

    // possible modifiers, used as a bitwise OR flag
    enum class keymod {
        none    = 0x0,
        shift   = 0x1,
        control = 0x2,
        alt     = 0x4,
        super   = 0x8,
        hyper   = 0x10
    };

    inline
    keymod operator | (keymod x, keymod y)
    {
        return static_cast<keymod>(bitflag_t(x) | bitflag_t(y));
    }
    inline
    keymod operator & (keymod x, keymod y)
    {
        return static_cast<keymod>((bitflag_t(x) & bitflag_t(y)));
    }
    inline
    void operator |= (keymod& x, keymod y)
    {
        x = x | y;
    }

    // primitive key symbols. this list represents a physical keys available
    // on most keyboards. the meaning of each raw key depends on the application.
    // key translation to characters is keyboard layout specific. Some keyboard
    // layouts have different mappings for different characters. For example with US 
    // layout double quote (") is a single key and to generate the same character with 
    // FI layout user needs to press Shift+2.
    // therefore this list does not contain any keys that are software layout specific.
    enum class keysym {
        none,                   // no known key
        backspace,
        tab,
        enter,
        space,
        // alphanumerics
        key_0, 
        key_1, 
        key_2, 
        key_3, 
        key_4, 
        key_5, 
        key_6,
        key_7, 
        key_8, 
        key_9,
        key_A, 
        key_B, 
        key_C, 
        key_D, 
        key_E, 
        key_F, 
        key_G, 
        key_H, 
        key_I, 
        key_J, 
        key_K, 
        key_L, 
        key_M, 
        key_N, 
        key_O, 
        key_P, 
        key_Q, 
        key_R, 
        key_S, 
        key_T, 
        key_U, 
        key_V, 
        key_W, 
        key_X, 
        key_Y,
        key_Z,
        // function keys
        f1, 
        f2, 
        f3, 
        f4, 
        f5, 
        f6, 
        f7, 
        f8, 
        f9,
        f10, 
        f11, 
        f12,
        // modifier keys
        control_R,
        control_L,
        shift_R,
        shift_L,
        alt_R,
        alt_L,
        capslock,
        // cursor control & motion
        insert,
        del,
        home,
        end,
        pageup,
        pagedown,
        // arrow keys,
        left,
        up,
        down,
        right,
        // numpad keys todo
        // other keys
        escape
    }; 


} // wdk


