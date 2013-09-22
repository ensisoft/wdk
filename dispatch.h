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

#include "display.h"
#include "event.h"
#include "ime.h"

namespace wdk
{
    namespace detail {
        template<typename T>
        bool dispatch(const wdk::event& ev, T& obj)
        {
            return obj.dispatch(ev);
        }

        bool dispatch(const wdk::event& ev, wdk::ime& im)
        {
            if (is_keyboard_event(ev.type))
            {
                im.add_input(ev);
                return false;
            }
            return im.dispatch(ev);
        }

    } //

template<typename T>
void dispatch_one(display& disp, T& win)
{
    wdk::event e = {0};
    disp.get_event(e);

    detail::dispatch(e, win);

    dispose(e);
}


template<typename T, typename F>
void dispatch_one(display& disp, T& win, F& keyb)
{
    wdk::event e = {0};
    disp.get_event(e);

    detail::dispatch(e, win);
    detail::dispatch(e, keyb);

    dispose(e);
}

template<typename T, typename F, typename X>
void dispatch_one(display& disp, T& win, F& keyb, X& im)
{
    wdk::event e = {0};
    disp.get_event(e);

    detail::dispatch(e, win);
    detail::dispatch(e, keyb);
    detail::dispatch(e, im);

    dispose(e);
}


template<typename T>
void dispatch_all(display& disp, T& win)
{
    while (disp.has_event())
    {
        wdk::event e = {0};
        disp.get_event(e);

        detail::dispatch(e, win);

        dispose(e);
    }
}

template<typename T, typename F>
void dispatch_all(display& disp, T& win, F& keyb)
{
    while (disp.has_event())
    {
        wdk::event e = {0};
        disp.get_event(e);

        detail::dispatch(e, win);
        detail::dispatch(e, keyb);

        dispose(e);
    }
}

template<typename T, typename F, typename X>
void dispatch_all(display& disp, T& win, F& keyb, X& im)
{
    while (disp.has_event())
    {
        wdk::event e = {0};
        disp.get_event(e);

        detail::dispatch(e, win);
        detail::dispatch(e, keyb);
        detail::dispatch(e, im);

        dispose(e);
    }
}

} // wdk
