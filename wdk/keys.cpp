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

std::string ToString(Keymod mod)
{
    assert(mod != Keymod::None);

    switch (mod)
    {
        case Keymod::None:    return "None";
        case Keymod::Shift:   return "Shift";
        case Keymod::Control: return "Ctrl";
        case Keymod::Alt:     return "Alt";
    }
    assert(0);
    return "";
}

std::string ToString(Keysym sym)
{
    assert(sym != Keysym::None);

    // this is not localized, but we don't care about it atm.
    switch (sym)
    {
        case Keysym::None:      return "None";
        case Keysym::Backspace: return "Backspace";
        case Keysym::Tab:       return "Tab";
        case Keysym::Enter:     return "Enter";
        case Keysym::Space:     return "Space";
        case Keysym::Key0:      return "0";
        case Keysym::Key1:      return "1";
        case Keysym::Key2:      return "2";
        case Keysym::Key3:      return "3";
        case Keysym::Key4:      return "4";
        case Keysym::Key5:      return "5";
        case Keysym::Key6:      return "6";
        case Keysym::Key7:      return "7";
        case Keysym::Key8:      return "8";        
        case Keysym::Key9:      return "9";                
        case Keysym::KeyA:      return "A";
        case Keysym::KeyB:      return "B";
        case Keysym::KeyC:      return "C";
        case Keysym::KeyD:      return "D";        
        case Keysym::KeyE:      return "E";
        case Keysym::KeyF:      return "F";        
        case Keysym::KeyG:      return "G";
        case Keysym::KeyH:      return "H";
        case Keysym::KeyI:      return "I";
        case Keysym::KeyJ:      return "J";                                
        case Keysym::KeyK:      return "K";                                
        case Keysym::KeyL:      return "L";                                
        case Keysym::KeyM:      return "M";                                
        case Keysym::KeyN:      return "N";                                
        case Keysym::KeyO:      return "O";
        case Keysym::KeyP:      return "P";
        case Keysym::KeyQ:      return "Q";
        case Keysym::KeyR:      return "R";
        case Keysym::KeyS:      return "S";                                
        case Keysym::KeyT:      return "T";
        case Keysym::KeyU:      return "U";        
        case Keysym::KeyV:      return "V";
        case Keysym::KeyW:      return "W";        
        case Keysym::KeyX:      return "X";        
        case Keysym::KeyY:      return "Y";                        
        case Keysym::KeyZ:      return "Z";                                
        case Keysym::F1:        return "F1";
        case Keysym::F2:        return "F2";
        case Keysym::F3:        return "F3";
        case Keysym::F4:        return "F4";
        case Keysym::F5:        return "F5";
        case Keysym::F6:        return "F6";
        case Keysym::F7:        return "F7";
        case Keysym::F8:        return "F8"; 
        case Keysym::F9:        return "F9";
        case Keysym::F10:       return "F10";
        case Keysym::F11:       return "F11";
        case Keysym::F12:       return "F12";        
        case Keysym::ControlR:  return "Ctrl_R";
        case Keysym::ControlL:  return "Ctrl_L";
        case Keysym::ShiftR:    return "Shift_R";
        case Keysym::ShiftL:    return "Shift_L";
        case Keysym::AltL:      return "Alt_L";
        case Keysym::CapsLock:  return "CapsLock";
        case Keysym::Insert:    return "Insert";
        case Keysym::Del:       return "Delete";
        case Keysym::Home:      return "Home";
        case Keysym::End:       return "End";
        case Keysym::PageUp:    return "Page Up";
        case Keysym::PageDown:  return "Page Down";
        case Keysym::ArrowLeft: return "Left";
        case Keysym::ArrowRight:return "Right";
        case Keysym::ArrowUp:   return "Up";
        case Keysym::ArrowDown: return "Down";
        case Keysym::Escape:    return "Esc";
        case Keysym::Plus:      return "Plus";
        case Keysym::Minus:     return "Minus";
    }
    assert(0);
    return "";
}

std::string ToString(MouseButton btn)
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
 
