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
    // interface for listening for window events
    class window_listener
    {
    public:
        virtual ~window_listener() {}
        virtual void on_create(const window_event_create&) {}
        virtual void on_paint(const window_event_paint&) {}
        virtual void on_resize(const window_event_resize&) {}
        virtual void on_lost_focus(const window_event_focus&) {}
        virtual void on_gain_focus(const window_event_focus&) {}
        virtual void on_query_close(window_event_query_close&) {}
        virtual void on_destroy(const window_event_destroy&) {}
    protected:
    private:
    };

    // connect all events in the window to the listener
    template<typename T>
    inline void connect(window& win, T& listener)
    {
        namespace args = std::placeholders;
        win.event_create      = std::bind(&window_listener::on_create, &listener, args::_1);
        win.event_paint       = std::bind(&window_listener::on_paint, &listener, args::_1);
        win.event_resize      = std::bind(&window_listener::on_resize, &listener, args::_1);
        win.event_lost_focus  = std::bind(&window_listener::on_lost_focus, &listener, args::_1);
        win.event_gain_focus  = std::bind(&window_listener::on_gain_focus, &listener, args::_1);
        win.event_query_close = std::bind(&window_listener::on_query_close, &listener, args::_1);
        win.event_destroy     = std::bind(&window_listener::on_destroy, &listener, args::_1);
    }

} // wdk
