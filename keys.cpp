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

std::string mod_name(keymod mod)
{
    assert(mod != keymod::none);

    switch (mod)
    {
        case keymod::shift:   return "Shift";
        case keymod::control: return "Ctrl";
        case keymod::alt:     return "Alt";
        case keymod::super:   return "Super";
        case keymod::hyper:   return "Hyper";
        case keymod::none:
            break;
    }
    assert(0);
    return "";
}

std::string key_name(keysym sym)
{
    assert(sym != keysym::none);

    // this is not localized, but we don't care about it atm.
    switch (sym)
    {
        case keysym::backspace: return "Backspace";
        case keysym::tab:       return "Tab";
        case keysym::enter:     return "Enter";
        case keysym::space:     return "Space";
        case keysym::key_0:     return "0";
        case keysym::key_1:     return "1";
        case keysym::key_2:     return "2";
        case keysym::key_3:     return "3";
        case keysym::key_4:     return "4";
        case keysym::key_5:     return "5";
        case keysym::key_6:     return "6";
        case keysym::key_7:     return "7";
        case keysym::key_8:     return "8";        
        case keysym::key_9:     return "9";                
        case keysym::key_A:     return "A";
        case keysym::key_B:     return "B";
        case keysym::key_C:     return "C";
        case keysym::key_D:     return "D";        
        case keysym::key_E:     return "E";
        case keysym::key_F:     return "F";        
        case keysym::key_G:     return "G";
        case keysym::key_H:     return "H";
        case keysym::key_I:     return "I";
        case keysym::key_J:     return "J";                                
        case keysym::key_K:     return "K";                                
        case keysym::key_L:     return "L";                                
        case keysym::key_M:     return "M";                                
        case keysym::key_N:     return "N";                                
        case keysym::key_O:     return "O";
        case keysym::key_P:     return "P";
        case keysym::key_Q:     return "Q";
        case keysym::key_R:     return "R";
        case keysym::key_S:     return "S";                                
        case keysym::key_T:     return "T";
        case keysym::key_U:     return "U";        
        case keysym::key_V:     return "V";
        case keysym::key_W:     return "W";        
        case keysym::key_X:     return "X";        
        case keysym::key_Y:     return "Y";                        
        case keysym::key_Z:     return "Z";                                
        case keysym::f1:        return "F1";
        case keysym::f2:        return "F2";
        case keysym::f3:        return "F3";
        case keysym::f4:        return "F4";
        case keysym::f5:        return "F5";
        case keysym::f6:        return "F6";
        case keysym::f7:        return "F7";
        case keysym::f8:        return "F8"; 
        case keysym::f9:        return "F9";
        case keysym::f10:       return "F10";
        case keysym::f11:       return "F11";
        case keysym::f12:       return "F12";        
        case keysym::control_R: return "Ctrl_R";
        case keysym::control_L: return "Ctrl_L";
        case keysym::shift_R:   return "Shift_R";
        case keysym::shift_L:   return "Shift_L";
        case keysym::alt_L:     return "Alt_L";
        case keysym::capslock:  return "CapsLock";
        case keysym::insert:    return "Insert";
        case keysym::del:       return "Delete";
        case keysym::home:      return "Home";
        case keysym::end:       return "End";
        case keysym::pageup:    return "Page Up";
        case keysym::pagedown:  return "Page Down";
        case keysym::left:      return "Left";
        case keysym::right:     return "Right";
        case keysym::up:        return "Up";
        case keysym::down:      return "Down";
        case keysym::escape:    return "Esc";
        case keysym::none:
            break;
    }
    assert(0);
    return "";
}

} // wdk
 
