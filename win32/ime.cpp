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

#include <windows.h>
#include <cassert>
#include "../utility/utf8.h"
#include "../types.h"
#include "../ime.h"
#include "../event.h"
#include "../events.h"

namespace wdk
{
struct ime::impl {
    ime::output output;
};

ime::ime(const display& disp, ime::output out) : pimpl_(new impl)
{
    pimpl_->output = out;
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
    // translate virtual key messages into unicode characters 
    // which are posted in WM_CHAR (UTF-16)
    // this works fine for BMP in the range 0x0000 - 0xD7FF, 0xE000 - 0xFFFF.
    // for values above 0xFFFF we should decode the UTF-16 to a single code point value
    // in order to be consistent with Linux implementation.
    return TranslateMessage(&ev.ev);     
}

bool ime::dispatch(const event& ev) const
{
    if (ev.type != event_type::ime_char)
        return false;

    if (event_char)
    {
        const WPARAM utf16 = ev.ev.wParam;

        ime_event_char e {0};

        if (pimpl_->output == ime::output::ascii)
        {
            e.ascii = utf16 & 0x7f;
        }
        else if (pimpl_->output == ime::output::ucs2)
        {
            e.ucs2 = utf16;
        }
        else if (pimpl_->output == ime::output::utf8)
        {
            enc::utf8_encode(&utf16, &utf16 + 1, &e.utf8[0]);
        }
        event_char(e);
    }

    return true;
}

} // wdk
