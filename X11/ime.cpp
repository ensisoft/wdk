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

#include <X11/keysym.h>
#include <cassert>
#include "../utility/utf8.h"
#include "../ime.h"
#include "../types.h"
#include "../event.h"
#include "../events.h"
#include "../display.h"

namespace linux {
    long keysym2ucs(KeySym keysym);
}// linux

namespace wdk
{
struct ime::impl {
    ime::output output;
    Atom atom_event_char;
    Display* disp;
};

ime::ime(const display& disp, ime::output out) : pimpl_(new impl)
{
    pimpl_->output = out;
    pimpl_->atom_event_char = XInternAtom(disp.handle(), "WDK_ATOM_EVENT_CHAR", False);
    pimpl_->disp = disp.handle();
}

ime::~ime()
{
}

ime::output ime::get_output() const
{
    return pimpl_->output;
}

void ime::set_output(ime::output out)
{
    pimpl_->output = out;
}

bool ime::add_input(const event& ev)
{
    if (ev.type != event_type::keyboard_keydown)
        return false;

    const XEvent* event = &ev.ev;

    KeySym sym = NoSymbol;

    XLookupString(const_cast<XKeyEvent*>(&event->xkey), nullptr, 0, &sym, nullptr);
    if (sym != NoSymbol)
    {
        const long ucs2 = linux::keysym2ucs(sym);
        if (ucs2 != -1)
        {
            // synthesize character event

            // this doesn't work as expected... maybe because the receiver window 
            // is the same window or because there's no event mask that we can specify..?
            // XEvent character = {0};
            // character.type              = ClientMessage;
            // character.xany.window       = event->xany.window;
            // character.xany.display      = event->xany.display;   
            // character.xclient.type      = 2;
            // character.xclient.format    = 32;
            // character.xclient.data.l[0] = ucs2;
            // character.xclient.message_type = pimpl_->atom_event_char;
            // const Status ret = XSendEvent(pimpl_->disp, event->xany.window, False, 0, &character);            
            // assert(ret);

            static_assert(sizeof(Window) >= sizeof(ucs2), "");

            XEvent hack = {0};
            hack.type = MapNotify;
            hack.xmap.window = ucs2;
            const Status ret = XSendEvent(event->xany.display, event->xany.window, False, 0, &hack);

            assert(ret);
        }            
    }
    return true;
}

bool ime::dispatch(const event& ev) const
{
    if (ev.type != event_type::ime_char)
        return false;

    if (event_char)
    {
        //const XClientMessageEvent* event = &ev.ev.xclient;

        //const long ucs2 = event->data.l[0];

        const XMapEvent* event = &ev.ev.xmap;

        const long ucs2 = event->window;

        ime_event_char e {0};

        if (pimpl_->output == ime::output::ascii)
        {
            e.ascii = ucs2 & 0x7f;
        }
        else if (pimpl_->output == ime::output::ucs2)
        {
            e.ucs2 = ucs2;
        }
        else if (pimpl_->output == ime::output::utf8)
        {
            enc::utf8_encode(&ucs2, &ucs2 + 1, &e.utf8[0]);
        }
        event_char(e);
    }
    return true;
}

} // wdk

