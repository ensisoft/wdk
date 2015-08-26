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

#include <algorithm>
#include <cassert>
#include "../system.h"
#include "../videomode.h"
#include "../keys.h"
#include "helpers.h"

namespace {
    using namespace wdk;

    struct key_mapping {
        keysym wdk;
        UINT   win;
    };
    bool operator<(const key_mapping& i, const key_mapping& j)
    {
        return i.wdk < j.wdk;
    }

    static key_mapping keymap[] = {
        {keysym::none,          0},
        {keysym::backspace,     VK_BACK},
        {keysym::tab,           VK_TAB},
        {keysym::enter,         VK_RETURN},
        {keysym::space,         VK_SPACE},
        {keysym::key_0,         0x30},
        {keysym::key_1,         0x31},
        {keysym::key_2,         0x32},
        {keysym::key_3,         0x33},
        {keysym::key_4,         0x34},
        {keysym::key_5,         0x35},
        {keysym::key_6,         0x36},
        {keysym::key_7,         0x37},
        {keysym::key_8,         0x38},
        {keysym::key_9,         0x39},
        {keysym::key_A,         0x41},
        {keysym::key_B,         0x42},
        {keysym::key_C,         0x43},
        {keysym::key_D,         0x44},
        {keysym::key_E,         0x45},
        {keysym::key_F,         0x46},
        {keysym::key_G,         0x47},
        {keysym::key_H,         0x48},
        {keysym::key_I,         0x49},
        {keysym::key_J,         0x4A},
        {keysym::key_K,         0x4B},
        {keysym::key_L,         0x4C},
        {keysym::key_M,         0x4D},
        {keysym::key_N,         0x4E},
        {keysym::key_O,         0x4F},
        {keysym::key_P,         0x50},
        {keysym::key_Q,         0x51},
        {keysym::key_R,         0x52},
        {keysym::key_S,         0x53},
        {keysym::key_T,         0x54},
        {keysym::key_U,         0x55},
        {keysym::key_V,         0x56},
        {keysym::key_W,         0x57},
        {keysym::key_X,         0x58},
        {keysym::key_Y,         0x59},
        {keysym::key_Z,         0x5A},
        {keysym::f1,            VK_F1},
        {keysym::f2,            VK_F2},
        {keysym::f3,            VK_F3},
        {keysym::f4,            VK_F4},
        {keysym::f5,            VK_F5},
        {keysym::f6,            VK_F6},
        {keysym::f7,            VK_F7},
        {keysym::f8,            VK_F8},
        {keysym::f9,            VK_F9},
        {keysym::f10,           VK_F10},
        {keysym::f11,           VK_F11},
        {keysym::f12,           VK_F12},
        {keysym::control_R,     VK_RCONTROL},
        {keysym::control_L,     VK_LCONTROL},
        {keysym::alt_L,         VK_LMENU},
        //{keysym::alt_R,         VK_RMENU},
        {keysym::shift_L,       VK_LSHIFT},
        {keysym::shift_R,       VK_RSHIFT},
        {keysym::capslock,      VK_CAPITAL},
        {keysym::insert,        VK_INSERT},
        {keysym::del,           VK_DELETE},
        {keysym::home,          VK_HOME},
        {keysym::end,           VK_END},
        {keysym::pageup,        VK_PRIOR},
        {keysym::pagedown,      VK_NEXT},
        {keysym::left,          VK_LEFT},
        {keysym::up,            VK_UP},
        {keysym::down,          VK_DOWN},
        {keysym::right,         VK_RIGHT},
        {keysym::escape,        VK_ESCAPE}
    };
    struct table_sorter {
        table_sorter() 
        {
            std::sort(std::begin(keymap), std::end(keymap));
        }
    } sort_my_data_bitch;

    UINT find_keysym(keysym sym)
    {
        assert(sym != keysym::none);

        const auto it = std::lower_bound(std::begin(keymap), std::end(keymap), key_mapping{sym, 0});

        // should be there
        assert(it != std::end(keymap));

        return (*it).win;
    }

    enum { KEY_DOWN = 0x8000 };

} // namespace

namespace wdk
{
native_display_t get_display_handle()
{
    struct desktop {
        HDC hdc;
        dummywin win;

        desktop() 
        {
            hdc = GetDC(NULL);
            win.bounce_display_change();
        }
        ~desktop() 
        {
            ReleaseDC(GetDesktopWindow(), hdc);
        }
    };

    static desktop d;

    return d.hdc;
}

videomode get_current_video_mode()
{
    DEVMODE cur_mode = {0};

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &cur_mode))
        throw std::runtime_error("failed to current display settings");

    videomode mode;
    mode.xres = cur_mode.dmPelsWidth;
    mode.yres = cur_mode.dmPelsHeight;
    return mode;

}

void set_video_mode(const videomode& m)
{
    DEVMODE mode = {0};
    mode.dmSize       = sizeof(mode);
    mode.dmPelsWidth  = m.xres;
    mode.dmPelsHeight = m.yres;
    mode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

    get_display_handle();

    if (ChangeDisplaySettings(&mode, 0) != DISP_CHANGE_SUCCESSFUL)
       throw std::runtime_error("display mode change failed");

}

std::vector<videomode> list_video_modes()
{
    std::vector<videomode> modes;

    DEVMODE dev;
    DWORD modeid = 0;

    while (EnumDisplaySettings(NULL, modeid++, &dev))
    {
        videomode mode(dev.dmPelsWidth, dev.dmPelsHeight);

        if (std::find(modes.begin(), modes.end(), mode) == modes.end())
            modes.push_back(mode);
    }

    return modes;
}

bool have_events()
{
    MSG m;

    return (PeekMessage(&m, NULL, 0, 0, PM_NOREMOVE) == TRUE);
}

bool sync_events()
{
    return false;
}

bool peek_event(native_event_t& ev)
{
    MSG m;

    if (!PeekMessage(&m, NULL, 0, 0, PM_NOREMOVE))
        return false;

    ev = native_event_t(m);

    return true;
}

native_event_t get_event()
{
    MSG m;

    GetMessage(&m, NULL, 0, 0);

    return native_event_t(m);

}

std::pair<bitflag<keymod>, keysym> translate_keydown_event(const native_event_t& key)
{
    const MSG& m = key;
    const uint_t native_keycode = m.wParam;

    std::pair<bitflag<keymod>, keysym> ret = {keymod::none, keysym::none};

    const auto it = std::find_if(std::begin(keymap), std::end(keymap), 
        [=](const key_mapping& map)
        {
            return map.win == native_keycode;
        });

    if (it != std::end(keymap))
    {
        const short alt_key   = GetKeyState(VK_MENU);
        const short shift_key = GetKeyState(VK_SHIFT);
        const short ctrl_key  = GetKeyState(VK_CONTROL);

        if (native_keycode != VK_MENU && (alt_key & KEY_DOWN))
            ret.first |= keymod::alt;
        if (native_keycode != VK_SHIFT && (shift_key & KEY_DOWN))
            ret.first |= keymod::shift;
        if (native_keycode != VK_CONTROL && (ctrl_key & KEY_DOWN))
            ret.first |= keymod::control;
        ret.second = (*it).wdk;
    }
    else if (native_keycode == VK_CONTROL)
    {
        if (GetKeyState(VK_LCONTROL) & KEY_DOWN)
            ret.second = keysym::control_L;
        else if (GetKeyState(VK_RCONTROL) & KEY_DOWN)
            ret.second = keysym::control_R;
    }
    else if (native_keycode == VK_MENU)
    {
        if (GetKeyState(VK_LMENU) & KEY_DOWN)
            ret.second = keysym::alt_L;
      /*  else if (GetAsyncKeyState(VK_RMENU) & KEY_DOWN)
            sym = keysym::alt_R;*/
    }
    else if (native_keycode == VK_SHIFT)
    {
       if (GetKeyState(VK_LSHIFT) & KEY_DOWN)
            ret.second = keysym::shift_L;
       else if (GetKeyState(VK_RSHIFT) & KEY_DOWN)
            ret.second = keysym::shift_R;
    }

    return ret;
}

bool test_key_down(keysym symbol)
{
    const UINT win    = find_keysym(symbol);
    const SHORT state = GetAsyncKeyState(win);

    return bool(state & KEY_DOWN);

}

bool test_key_down(uint_t keycode)
{
    assert(keycode);

    const SHORT state = GetAsyncKeyState(keycode);

    return bool(state & KEY_DOWN);
}

uint_t keysym_to_keycode(keysym symbol)
{
    const UINT win = find_keysym(symbol);

    return win;
}

} // wdk
