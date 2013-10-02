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

namespace wdk
{
    // interface for listening for window events
    class event_listener
    {
    public:
        virtual ~event_listener() {}
        virtual void on_create(const window_event_create&) {}
        virtual void on_paint(const window_event_paint&) {}
        virtual void on_resize(const window_event_resize&) {}
        virtual void on_lost_focus(const window_event_focus&) {}
        virtual void on_gain_focus(const window_event_focus&) {}
        virtual void on_want_close(const window_event_want_close&) {}
        virtual void on_keydown(const window_event_keydown&) {}
        virtual void on_keyup(const window_event_keyup&) {}
        virtual void on_char(const window_event_char&) {}
    protected:
    private:
    };


    // connect all events in the window to the listener
    template<typename T>
    inline void connect(window& win, T& list)
    {
        namespace args = std::placeholders;

        win.on_create     = std::bind(&event_listener::on_create, &list, args::_1);
        win.on_paint      = std::bind(&event_listener::on_paint, &list, args::_1);
        win.on_resize     = std::bind(&event_listener::on_resize, &list, args::_1);
        win.on_lost_focus = std::bind(&event_listener::on_lost_focus, &list, args::_1);
        win.on_gain_focus = std::bind(&event_listener::on_gain_focus, &list, args::_1);
        win.on_want_close = std::bind(&event_listener::on_want_close, &list, args::_1);
        win.on_keydown    = std::bind(&event_listener::on_keydown, &list, args::_1);
        win.on_keyup      = std::bind(&event_listener::on_keyup, &list, args::_1);
        win.on_char       = std::bind(&event_listener::on_char, &list, args::_1);
    }

} // wdk
