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

#include <functional>   // for function, bind
#include <memory>       // for unique_ptr
#include <utility>      // for pair
#include "keys.h"
#include "types.h"
#include "utility.h"
#include "fwddecl.h"

namespace wdk
{
    // keyboard class allows to translate native keycodes and events
    // to virtual key/modifier pairs defined in the library based on the current
    // keyboard state. if also event dispatching the input is translated
    // into unicode character input.
    class keyboard : noncopyable
    {
    public:
        // bind these to listen to various events
        std::function<void (const keyboard_event_keypress&)> event_keyup;
        std::function<void (const keyboard_event_keypress&)> event_keydown;

        keyboard(const display& disp);
       ~keyboard();

        // name a modifier key
        std::string name(keymod modifier) const;
        
        // name a keysym
        std::string name(keysym symbol) const;

        // return the name of the native key pressed (engraving on the key board)
        std::string name(uint_t native_keycode) const;

        // translate keyboard event to virtual keysym/modifier pair
        std::pair<keymod, keysym> translate(const event& ev) const;

        // translate native key values to virtual keysym/modifier pair
        std::pair<keymod, keysym> translate(uint_t native_modifier, uint_t native_keycode) const;

        // dispatch the given event
        bool dispatch(const event& ev) const;
    private:
        struct impl;
        
        std::unique_ptr<impl> pimpl_;
        
    };

} // wdk
