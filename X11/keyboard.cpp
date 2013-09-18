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

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include <iterator>
#include "../keyboard.h"
#include "../events.h"
#include "../event.h"

namespace linux {
    long keysym2ucs(KeySym keysym);
}// linux

namespace {
    using namespace wdk;
    
    struct key_mapping {
        wdk::keysym wdk;
        KeySym x11;
    };
    bool operator<(const key_mapping& i, const key_mapping& j)
    {
        return i.x11 < j.x11;
    }

    static key_mapping keymap[] = {
        {keysym::none,                  NoSymbol},
        {keysym::backspace,             XK_BackSpace},
        {keysym::tab,                   XK_Tab},
        {keysym::enter,                 XK_Return},
        {keysym::space,                 XK_space},
        {keysym::key_0,                 XK_0},
        {keysym::key_1,                 XK_1},
        {keysym::key_2,                 XK_2},
        {keysym::key_3,                 XK_3},
        {keysym::key_4,                 XK_4},
        {keysym::key_5,                 XK_5},
        {keysym::key_6,                 XK_6},
        {keysym::key_7,                 XK_7},
        {keysym::key_8,                 XK_8},
        {keysym::key_9,                 XK_9},
        {keysym::key_A,                 XK_a},
        {keysym::key_B,                 XK_b},
        {keysym::key_C,                 XK_c},
        {keysym::key_D,                 XK_d},
        {keysym::key_E,                 XK_e},
        {keysym::key_F,                 XK_f},
        {keysym::key_G,                 XK_g},
        {keysym::key_H,                 XK_h},
        {keysym::key_I,                 XK_i},
        {keysym::key_J,                 XK_j},
        {keysym::key_K,                 XK_k},
        {keysym::key_L,                 XK_l},
        {keysym::key_M,                 XK_m},
        {keysym::key_N,                 XK_n},
        {keysym::key_O,                 XK_o},
        {keysym::key_P,                 XK_p},
        {keysym::key_Q,                 XK_q},
        {keysym::key_R,                 XK_r},
        {keysym::key_S,                 XK_s},
        {keysym::key_T,                 XK_t},
        {keysym::key_U,                 XK_u},
        {keysym::key_V,                 XK_v},
        {keysym::key_W,                 XK_w},
        {keysym::key_X,                 XK_x},
        {keysym::key_Y,                 XK_y},
        {keysym::key_Z,                 XK_z},

        {keysym::key_A,                 XK_A},
        {keysym::key_B,                 XK_B},
        {keysym::key_C,                 XK_C},
        {keysym::key_D,                 XK_D},
        {keysym::key_E,                 XK_E},
        {keysym::key_F,                 XK_F},
        {keysym::key_G,                 XK_G},
        {keysym::key_H,                 XK_H},
        {keysym::key_I,                 XK_I},
        {keysym::key_J,                 XK_J},
        {keysym::key_K,                 XK_K},
        {keysym::key_L,                 XK_L},
        {keysym::key_M,                 XK_M},
        {keysym::key_N,                 XK_N},
        {keysym::key_O,                 XK_O},
        {keysym::key_P,                 XK_P},
        {keysym::key_Q,                 XK_Q},
        {keysym::key_R,                 XK_R},
        {keysym::key_S,                 XK_S},
        {keysym::key_T,                 XK_T},
        {keysym::key_U,                 XK_U},
        {keysym::key_V,                 XK_V},
        {keysym::key_W,                 XK_W},
        {keysym::key_X,                 XK_X},
        {keysym::key_Y,                 XK_Y},
        {keysym::key_Z,                 XK_Z},

        {keysym::f1,                    XK_F1},
        {keysym::f2,                    XK_F2},
        {keysym::f3,                    XK_F3},
        {keysym::f4,                    XK_F4},
        {keysym::f5,                    XK_F5},
        {keysym::f6,                    XK_F6},
        {keysym::f7,                    XK_F7},
        {keysym::f8,                    XK_F8},
        {keysym::f9,                    XK_F9},
        {keysym::f10,                   XK_F10},
        {keysym::f11,                   XK_F11},
        {keysym::f12,                   XK_F12},
        {keysym::control_L,             XK_Control_L},
        {keysym::control_R,             XK_Control_R},
        {keysym::alt_L,                 XK_Alt_L},
        {keysym::alt_R,                 XK_Alt_R},
        {keysym::capslock,              XK_Caps_Lock},
        {keysym::shift_L,               XK_Shift_L},
        {keysym::shift_R,               XK_Shift_R},
        {keysym::insert,                XK_Insert},
        {keysym::del,                   XK_Delete},
        {keysym::home,                  XK_Home},
        {keysym::pageup,                XK_Page_Up},
        {keysym::pagedown,              XK_Page_Down},
        {keysym::left,                  XK_Left},
        {keysym::right,                 XK_Right},
        {keysym::down,                  XK_Down},
        {keysym::up,                    XK_Up},
        {keysym::escape,                XK_Escape},

        // why does Right ALT Generate XK_ISO_Level3_Shift
        // instead of XK_Alt_R ??
        //{keysym::alt_R,                 XK_ISO_Level3_Shift}
    };
    


} // namespace

namespace wdk
{

struct keyboard::impl {
    Display* dpy;
    uint_t alt_mask;
};


keyboard::keyboard(native_display_t disp)
{
    assert(disp);

    pimpl_.reset(new impl);
    pimpl_->dpy      = disp;
    pimpl_->alt_mask = 0;

    // get the modifier map for finding XK_Alt_L or XK_Alt_R
    XModifierKeymap* mods = XGetModifierMapping(pimpl_->dpy);

    // there's a maximum of 8 modifiers in X server.
    // (Shift, Alt, Control, Meta, Super, Hyper, ModeSwitch, NumLock)
    // but a server can support a variable number of keys 
    // being assigned to any given modifier (max_keypermod)
    // Alt is the one that we try to find here through XK_Alt_L or XK_Alt_R.
    // Other interesting modifiers, control and shift are constants
    for (int mod=0; mod<8; ++mod)
    {
        for (int key=0; key<mods->max_keypermod; ++key)
        {
            const KeyCode code = mods->modifiermap[mod * mods->max_keypermod + key];
            KeySym  sym  = NoSymbol;
            int group    = 0;
            do 
            {
                sym = XKeycodeToKeysym(pimpl_->dpy, code, group++);
            } 
            while (sym == NoSymbol && group < 4);

            if (sym == XK_Alt_L)
            {
                pimpl_->alt_mask |= (1 << mod);
                //printf("Found LeftAlt mask %x", (1<<mod));
            }
            else if (sym == XK_Alt_R)
            {
                pimpl_->alt_mask |= (1 << mod);
                //printf("Found RightAlt mask %x", (1<<mod));
            }
        }
    }
    
    XFreeModifiermap(mods);

    // sort the mapping table by X11 values for quicker lookup
    std::sort(std::begin(keymap), std::end(keymap));
}

keyboard::~keyboard()
{
}

std::string keyboard::name(keymod m)
{
    switch (m)
    {
        case keymod::shift:   return "Shift";
        case keymod::control: return "Ctrl";
        case keymod::alt:     return "Alt";
        case keymod::super:   return "Super";
        case keymod::hyper:   return "Hyper";
        default:          
            break;
    }
    return "";
}

std::string keyboard::name(keysym s)
{
    if (s == keysym::none)
        return "";

    // have to do a linear search since the keyboard mapping table 
    // is sorted by X11 values.
    const auto it = std::find_if(std::begin(keymap), std::end(keymap),
        [=] (const key_mapping& map)
        {
            return map.wdk == s;
        });

    if (it == std::end(keymap))
        return "";

    const char* name = XKeysymToString((*it).x11);
    if (!name)
        return "";

    return std::string(name);
}

std::string keyboard::name(uint_t native_keycode)
{
    // todo: what the hell is the index?? (last argument)
    const KeySym sym = XKeycodeToKeysym(pimpl_->dpy, native_keycode, 0);
    if (sym == NoSymbol)
        return "";
    const char* name = XKeysymToString(sym);
    if (!name)
        return "";
    return std::string(name);
}

std::pair<keymod, keysym> keyboard::translate(event& ev)
{
    const XEvent* xev = &ev.ev;
    if (xev->type != KeyPress && xev->type != KeyRelease)
        std::pair<keymod, keysym>{keymod::none, keysym::none};

    const uint_t mod  = xev->xkey.state;
    const uint_t sym  = xev->xkey.keycode;

    return translate(mod, sym);
}

std::pair<keymod, keysym> keyboard::translate(uint_t native_modifier, uint_t native_keycode)
{
    std::pair<keymod, keysym> ret {keymod::none, keysym::none};

    XKeyEvent xev  = {0};
    xev.display    = pimpl_->dpy;
    xev.type       = KeyPress;
    xev.keycode    = native_keycode;
    // disregard modifiers for symbol lookup, only want to look up the keysym,
    // not X's idea of translated keysym+modifier
    //ev.state     = native_modifier;
    KeySym sym = NoSymbol;
    XLookupString(&xev, NULL, 0, &sym, NULL);
    if (sym == NoSymbol)
        return ret;

    const auto it = std::lower_bound(std::begin(keymap), std::end(keymap), key_mapping{keysym::none, sym});
    if (it == std::end(keymap) || (*it).x11 != sym)
        return ret;

    ret.second = (*it).wdk;
    if (native_modifier & pimpl_->alt_mask)
        ret.first |= keymod::alt;
    if (native_modifier & ControlMask)
        ret.first |= keymod::control;
    if (native_modifier & ShiftMask)
        ret.first |= keymod::shift;

    return ret;
}

bool keyboard::dispatch_event(event& ev)
{
    XEvent* event = &ev.ev;

    if (ev.type == event_type::keyboard_keydown)
    {
        if (event_keydown)
        {
            const auto key = translate(ev);
            if (key.second != keysym::none)            
                event_keydown(keyboard_event_keydown{key.second, key.first});
        }
        if (event_char)
        {
            KeySym sym = NoSymbol;
            XLookupString(&event->xkey, nullptr, 0, &sym, nullptr);
            if (sym != NoSymbol)
            {
                const long ucs2 = linux::keysym2ucs(sym);
                if (ucs2 != -1)
                    event_char(keyboard_event_char{ucs2});
            }
        }
        return true;
    }
    else if (ev.type == event_type::keyboard_keyup && event_keyup)
    {
        const auto key = translate(ev);

        if (key.second != keysym::none)
            event_keyup(keyboard_event_keyup{key.second, key.first});            
        return true;
    }
    return false;
}


} // wdk

