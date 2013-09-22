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

#include <functional>
#include <memory>
#include "utility.h"
#include "fwddecl.h"

namespace wdk
{
    // simplistic input method wrapper for 
    // handling translated (aka cooked) character input
    class ime : noncopyable
    {
    public:
        std::function<void (const ime_event_char&)> event_char;

        // possible translated output format
        enum class output { ascii, ucs2, utf8 };

        ime(const display& disp, output out = output::ucs2);

       ~ime();

        output get_output() const;

        void set_output(output out);

        // add new keyboard event into the current input state.
        // once input characters are ready they are posted 
        // in the display's event queue for the window in question.
        bool add_input(const event& ev);

        // dispatch character event
        bool dispatch(const event& ev) const;
   private:
       struct impl;

       std::unique_ptr<impl> pimpl_;
    }; 
} // wdk

