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

#include <cassert>
#include "keys.h"

namespace wdk
{

std::string GetModifierName(KeyModifier mod)
{
    assert(mod != KeyModifier::None);

    switch (mod)
    {
        case KeyModifier::None:    return "None";
        case KeyModifier::Shift:   return "Shift";
        case KeyModifier::Control: return "Ctrl";
        case KeyModifier::Alt:     return "Alt";
    }
    assert(0);
    return "";
}

std::string GetKeySymbolName(KeySymbol sym)
{
    assert(sym != KeySymbol::None);

    // this is not localized, but we don't care about it atm.
    switch (sym)
    {
        case KeySymbol::None:      return "None";
        case KeySymbol::Backspace: return "Backspace";
        case KeySymbol::Tab:       return "Tab";
        case KeySymbol::Enter:     return "Enter";
        case KeySymbol::Space:     return "Space";
        case KeySymbol::Key_0:     return "0";
        case KeySymbol::Key_1:     return "1";
        case KeySymbol::Key_2:     return "2";
        case KeySymbol::Key_3:     return "3";
        case KeySymbol::Key_4:     return "4";
        case KeySymbol::Key_5:     return "5";
        case KeySymbol::Key_6:     return "6";
        case KeySymbol::Key_7:     return "7";
        case KeySymbol::Key_8:     return "8";        
        case KeySymbol::Key_9:     return "9";                
        case KeySymbol::Key_A:     return "A";
        case KeySymbol::Key_B:     return "B";
        case KeySymbol::Key_C:     return "C";
        case KeySymbol::Key_D:     return "D";        
        case KeySymbol::Key_E:     return "E";
        case KeySymbol::Key_F:     return "F";        
        case KeySymbol::Key_G:     return "G";
        case KeySymbol::Key_H:     return "H";
        case KeySymbol::Key_I:     return "I";
        case KeySymbol::Key_J:     return "J";                                
        case KeySymbol::Key_K:     return "K";                                
        case KeySymbol::Key_L:     return "L";                                
        case KeySymbol::Key_M:     return "M";                                
        case KeySymbol::Key_N:     return "N";                                
        case KeySymbol::Key_O:     return "O";
        case KeySymbol::Key_P:     return "P";
        case KeySymbol::Key_Q:     return "Q";
        case KeySymbol::Key_R:     return "R";
        case KeySymbol::Key_S:     return "S";                                
        case KeySymbol::Key_T:     return "T";
        case KeySymbol::Key_U:     return "U";        
        case KeySymbol::Key_V:     return "V";
        case KeySymbol::Key_W:     return "W";        
        case KeySymbol::Key_X:     return "X";        
        case KeySymbol::Key_Y:     return "Y";                        
        case KeySymbol::Key_Z:     return "Z";                                
        case KeySymbol::F1:        return "F1";
        case KeySymbol::F2:        return "F2";
        case KeySymbol::F3:        return "F3";
        case KeySymbol::F4:        return "F4";
        case KeySymbol::F5:        return "F5";
        case KeySymbol::F6:        return "F6";
        case KeySymbol::F7:        return "F7";
        case KeySymbol::F8:        return "F8"; 
        case KeySymbol::F9:        return "F9";
        case KeySymbol::F10:       return "F10";
        case KeySymbol::F11:       return "F11";
        case KeySymbol::F12:       return "F12";        
        case KeySymbol::Control_R: return "Ctrl_R";
        case KeySymbol::Control_L: return "Ctrl_L";
        case KeySymbol::Shift_R:   return "Shift_R";
        case KeySymbol::Shift_L:   return "Shift_L";
        case KeySymbol::Alt_L:     return "Alt_L";
		case KeySymbol::CapsLock:  return "CapsLock";
        case KeySymbol::Insert:    return "Insert";
        case KeySymbol::Delete:    return "Delete";
        case KeySymbol::Home:      return "Home";
        case KeySymbol::End:       return "End";
        case KeySymbol::PageUp:    return "Page Up";
        case KeySymbol::PageDown:  return "Page Down";
        case KeySymbol::ArrowLeft: return "Left";
        case KeySymbol::ArrowRight: return "Right";
        case KeySymbol::ArrowUp:   return "Up";
        case KeySymbol::ArrowDown: return "Down";
        case KeySymbol::Escape:    return "Esc";
    }
    assert(0);
    return "";
}

std::string GetMouseButtonName(MouseButton btn)
{
    switch (btn)
    {
        case MouseButton::None:            return "None";
        case MouseButton::Left:            return "Left";
        case MouseButton::Right:           return "Right";
        case MouseButton::Wheel:           return "Wheel";
        case MouseButton::WheelScrollUp:   return "Wheel Up";
        case MouseButton::WheelScrollDown: return "Wheel Down";
        case MouseButton::Thumb1:          return "Thumb 1";
        case MouseButton::Thumb2:          return "Thumb 2";
        case MouseButton::Thumb3:          return "Thumb 3";
        case MouseButton::Thumb4:          return "Thumb 4";
    }
    assert(0);
    return "";
}

} // wdk
 
