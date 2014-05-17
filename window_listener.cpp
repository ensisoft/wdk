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
#include "window_listener.h"
#include "window_events.h"
#include "window.h"

namespace wdk
{

void connect(wdk::window& window, wdk::window_listener& listener)
{
    namespace args = std::placeholders;

    window.on_create     = std::bind(&window_listener::on_create, &listener, args::_1);
    window.on_paint      = std::bind(&window_listener::on_paint, &listener, args::_1);
    window.on_resize     = std::bind(&window_listener::on_resize, &listener, args::_1);
    window.on_lost_focus = std::bind(&window_listener::on_lost_focus, &listener, args::_1);
    window.on_gain_focus = std::bind(&window_listener::on_gain_focus, &listener, args::_1);
    window.on_want_close = std::bind(&window_listener::on_want_close, &listener, args::_1);
    window.on_keydown    = std::bind(&window_listener::on_keydown, &listener, args::_1);
    window.on_keyup      = std::bind(&window_listener::on_keyup, &listener, args::_1);
    window.on_char       = std::bind(&window_listener::on_char, &listener, args::_1);
}

} // wdk

