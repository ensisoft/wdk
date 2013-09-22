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

#include <functional>
#include "fwddecl.h"

namespace wdk
{
    // interface for listening for keyboard events
    class keyboard_listener 
    {
    public:
        virtual ~keyboard_listener() {}
        virtual void on_keyup(const keyboard_event_keypress&) {}
        virtual void on_keydown(const keyboard_event_keypress&) {}
    protected:
    private:
    };

    // connect all events in the keyboard to the listener
    template<typename T>
    inline void connect(keyboard& kb, T& listener)
    {
        namespace args = std::placeholders;
        kb.event_keyup   = std::bind(&keyboard_listener::on_keyup, listener, args::_1);
        kb.event_keydown = std::bind(&keyboard_listener::on_keydown, listener, args::_1);
    }    

} // wdk
