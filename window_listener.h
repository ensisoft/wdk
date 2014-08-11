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

namespace wdk
{
    struct window_event_create;
    struct window_event_paint;
    struct window_event_resize;
    struct window_event_focus;
    struct window_event_want_close;
    struct window_event_keyup;
    struct window_event_keydown;    
    struct window_event_char;
    struct window_event_mouse_move;
    struct window_event_mouse_press;
    struct window_event_mouse_release;
    class  window;

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
        virtual void on_want_close(const window_event_want_close&) {}
        virtual void on_keydown(const window_event_keydown&) {}
        virtual void on_keyup(const window_event_keyup&) {}
        virtual void on_char(const window_event_char&) {}
        virtual void on_mouse_move(const window_event_mouse_move&) {}
        virtual void on_mouse_press(const window_event_mouse_press&) {}
        virtual void on_mouse_release(const window_event_mouse_release&) {}
    protected:
    private:
    };

    // connect all events in the window to the listener
    void connect(wdk::window& window, wdk::window_listener& listener);

} // wdk

