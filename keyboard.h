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

namespace wdk
{

    // native window system event
    struct event;

    // keyboard events
    struct keyboard_event_keypress;
    struct keyboard_event_char;


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
        std::function<void (const keyboard_event_char&)>     event_char;

        keyboard(native_display_t disp);
       ~keyboard();

        // name a modifier key
        std::string name(keymod modifier);
        
        // name a keysym
        std::string name(keysym symbol);

        // return the name of the native key pressed (engraving on the key board)
        std::string name(uint_t native_keycode);

        // translate keyboard event to virtual keysym/modifier pair
        std::pair<keymod, keysym> translate(event& ev);

        // translate native key values to virtual keysym/modifier pair
        std::pair<keymod, keysym> translate(uint_t native_modifier, uint_t native_keycode);

        // dispatch the given event
        bool dispatch_event(event& ev);
    private:
        struct impl;
        
        std::unique_ptr<impl> pimpl_;
        
    };

    // interface for listening to keyboard events
    class keyboard_listener 
    {
    public:
        virtual ~keyboard_listener() {}
        virtual void on_keyup(const keyboard_event_keypress&) {}
        virtual void on_keydown(const keyboard_event_keypress&) {}
        virtual void on_char(const keyboard_event_char&) {}
    protected:
    private:
    };

    // connect all events in the given keyboard to the given listener
    inline void connect(keyboard& kb, keyboard_listener* listener)
    {
        namespace args = std::placeholders;
        kb.event_keyup   = std::bind(&keyboard_listener::on_keyup, listener, args::_1);
        kb.event_keydown = std::bind(&keyboard_listener::on_keydown, listener, args::_1);
        kb.event_char    = std::bind(&keyboard_listener::on_char, listener, args::_1);
    }

} // wdk
