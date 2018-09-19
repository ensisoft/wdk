// Copyright (c) 2018 Sami Väisänen, Ensisoft 
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


namespace wdk
{

    // available mouse buttons. a "standard" mouse is expected to have 
    // left and right buttons and a mouse wheel. 
    // some mice also have more buttons in "non-standard" locations
    // these are named thumbN buttons here.
    enum class MouseButton 
	{
        None, 
		Left, 
		Right, 
		Wheel, 
		WheelScrollUp, 
		WheelScrollDown, 
        Thumb1, 
		Thumb2, 
		Thumb3, 
		Thumb4
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
    enum class KeyModifier 
	{
        None, 
		Shift,
		Control,
		Alt
    };

    // primitive key symbols. this list represents a physical keys available
    // on most keyboards. the meaning of each raw key depends on the application.
    // key translation to characters is keyboard layout specific. Some keyboard
    // layouts have different mappings for different characters. For example with US 
    // layout double quote (") is a single key and to generate the same character with 
    // FI layout user needs to press Shift+2.
    // therefore this list does not contain any keys that are layout specific but maps
    // directly to physical keys on the keyboard
	enum class KeySymbol
	{
		None,                   // no known key
		Backspace,
		Tab,
		Enter,
		Space,
		Key_0,
		Key_1,
		Key_2,
		Key_3,
		Key_4,
		Key_5,
		Key_6,
		Key_7,
		Key_8,
		Key_9,
		Key_A,
		Key_B,
		Key_C,
		Key_D,
		Key_E,
		Key_F,
		Key_G,
		Key_H,
		Key_I,
		Key_J,
		Key_K,
        Key_L, 
        Key_M, 
        Key_N, 
        Key_O, 
        Key_P, 
        Key_Q, 
        Key_R, 
        Key_S, 
        Key_T, 
        Key_U, 
        Key_V, 
        Key_W, 
        Key_X, 
        Key_Y,
        Key_Z,
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
        Control_R,
        Control_L,
        Shift_R,
        Shift_L,
        Alt_L,
        CapsLock,
        Insert,
        Delete,
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

	// return the English modifier key name. 
    std::string GetModifierName(KeyModifier mod);

	// return the English key symbol name.
    std::string GetKeySymbolName(KeySymbol sym);

	// return the English mouse button name.
    std::string GetMouseButtonName(MouseButton btn);

} // wdk


