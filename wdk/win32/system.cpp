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

namespace {
    using namespace wdk;

    struct key_mapping {
        KeySymbol wdk;
        UINT   win;
    };
    bool operator<(const key_mapping& i, const key_mapping& j)
    {
        return i.wdk < j.wdk;
    }

    static key_mapping keymap[] = {
        {KeySymbol::None,          0},
        {KeySymbol::Backspace,     VK_BACK},
        {KeySymbol::Tab,           VK_TAB},
        {KeySymbol::Enter,         VK_RETURN},
        {KeySymbol::Space,         VK_SPACE},
        {KeySymbol::Key_0,         0x30},
        {KeySymbol::Key_1,         0x31},
        {KeySymbol::Key_2,         0x32},
        {KeySymbol::Key_3,         0x33},
        {KeySymbol::Key_4,         0x34},
        {KeySymbol::Key_5,         0x35},
        {KeySymbol::Key_6,         0x36},
        {KeySymbol::Key_7,         0x37},
        {KeySymbol::Key_8,         0x38},
        {KeySymbol::Key_9,         0x39},
        {KeySymbol::Key_A,         0x41},
        {KeySymbol::Key_B,         0x42},
        {KeySymbol::Key_C,         0x43},
        {KeySymbol::Key_D,         0x44},
        {KeySymbol::Key_E,         0x45},
        {KeySymbol::Key_F,         0x46},
        {KeySymbol::Key_G,         0x47},
        {KeySymbol::Key_H,         0x48},
        {KeySymbol::Key_I,         0x49},
        {KeySymbol::Key_J,         0x4A},
        {KeySymbol::Key_K,         0x4B},
        {KeySymbol::Key_L,         0x4C},
        {KeySymbol::Key_M,         0x4D},
        {KeySymbol::Key_N,         0x4E},
        {KeySymbol::Key_O,         0x4F},
        {KeySymbol::Key_P,         0x50},
        {KeySymbol::Key_Q,         0x51},
        {KeySymbol::Key_R,         0x52},
        {KeySymbol::Key_S,         0x53},
        {KeySymbol::Key_T,         0x54},
        {KeySymbol::Key_U,         0x55},
        {KeySymbol::Key_V,         0x56},
        {KeySymbol::Key_W,         0x57},
        {KeySymbol::Key_X,         0x58},
        {KeySymbol::Key_Y,         0x59},
        {KeySymbol::Key_Z,         0x5A},
        {KeySymbol::F1,            VK_F1},
        {KeySymbol::F2,            VK_F2},
        {KeySymbol::F3,            VK_F3},
        {KeySymbol::F4,            VK_F4},
        {KeySymbol::F5,            VK_F5},
        {KeySymbol::F6,            VK_F6},
        {KeySymbol::F7,            VK_F7},
        {KeySymbol::F8,            VK_F8},
        {KeySymbol::F9,            VK_F9},
        {KeySymbol::F10,           VK_F10},
        {KeySymbol::F11,           VK_F11},
        {KeySymbol::F12,           VK_F12},
        {KeySymbol::Control_R,     VK_RCONTROL},
        {KeySymbol::Control_L,     VK_LCONTROL},
        {KeySymbol::Alt_L,         VK_LMENU},
        //{keysym::alt_R,         VK_RMENU},
        {KeySymbol::Shift_L,       VK_LSHIFT},
        {KeySymbol::Shift_R,       VK_RSHIFT},
        {KeySymbol::CapsLock,      VK_CAPITAL},
        {KeySymbol::Insert,        VK_INSERT},
        {KeySymbol::Delete,        VK_DELETE},
        {KeySymbol::Home,          VK_HOME},
        {KeySymbol::End,           VK_END},
        {KeySymbol::PageUp,        VK_PRIOR},
        {KeySymbol::PageDown,      VK_NEXT},
        {KeySymbol::ArrowLeft,     VK_LEFT},
        {KeySymbol::ArrowUp,       VK_UP},
        {KeySymbol::ArrowDown,     VK_DOWN},
        {KeySymbol::ArrowRight,    VK_RIGHT},
        {KeySymbol::Escape,        VK_ESCAPE}
    };
    struct table_sorter {
        table_sorter()
        {
            std::sort(std::begin(keymap), std::end(keymap));
        }
    } sort_my_data_bitch;

    UINT find_keysym(KeySymbol sym)
    {
        assert(sym != KeySymbol::None);

        const auto it = std::lower_bound(std::begin(keymap), std::end(keymap), key_mapping{sym, 0});

        // should be there
        assert(it != std::end(keymap));

        return (*it).win;
    }

    enum { KEY_DOWN = 0x8000 };

} // namespace

namespace wdk
{
native_display_t GetNativeDisplayHandle()
{
    class Display
    {
    public:
        Display()
        {
            WNDCLASSEX cls    = {0};
            cls.cbSize        = sizeof(cls);
            cls.lpfnWndProc   = DefWindowProc;
            cls.lpszClassName = TEXT("WDK-SYSTEM-WINDOW");
            RegisterClassEx(&cls);

            // In order to detect the display change notification we need
            // to have a valid window. We don't know if the user has created
            // any windows or not, so we'll create one here just to listen
            // for that message.
            m_wnd_system = CreateWindow(TEXT("WDK-SYSTEM-WINDOW"), TEXT(""),
                WS_POPUP, 0, 0, 1, 1,
                NULL, NULL, NULL, NULL);
            if (m_wnd_system == NULL)
                throw std::runtime_error("create system window failed");

            SetWindowLongPtr(m_wnd_system, GWLP_WNDPROC,
                (LONG_PTR)Display::WndProc);

            m_hdc_desktop = GetDC(NULL);

        }
       ~Display()
        {
            BOOL ret = TRUE;
            ret = ReleaseDC(GetDesktopWindow(), m_hdc_desktop);
            assert(ret == TRUE);
            ret = DestroyWindow(m_wnd_system);
            (void)ret;
        }
        HDC getDesktopHDC() const
        { return m_hdc_desktop; }
    private:
        static
        LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
        {
            if (msg != WM_DISPLAYCHANGE)
                return DefWindowProc(hwnd, msg, wp, lp);
            PostMessage(hwnd, WM_DISPLAYCHANGE, wp, lp);
            return 0;
        }

    private:
        HDC   m_hdc_desktop;
        HWND  m_wnd_system;
    };

    static Display disp;

    return disp.getDesktopHDC();
}

VideoMode GetCurrentVideoMode()
{
    DEVMODE cur_mode = {0};

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &cur_mode))
        throw std::runtime_error("failed to current display settings");

    VideoMode mode;
    mode.xres = cur_mode.dmPelsWidth;
    mode.yres = cur_mode.dmPelsHeight;
    return mode;

}

void SetVideoMode(const VideoMode& m)
{
    DEVMODE mode = {0};
    mode.dmSize       = sizeof(mode);
    mode.dmPelsWidth  = m.xres;
    mode.dmPelsHeight = m.yres;
    mode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

    // Call this function here so that we actually create
    // our window to listen for the notification about display change.
    // todo: should we be able to receive this notification *always*
    // without having to initiate display change ourselves?
    GetNativeDisplayHandle();

    if (ChangeDisplaySettings(&mode, 0) != DISP_CHANGE_SUCCESSFUL)
       throw std::runtime_error("display mode change failed");

}

std::vector<VideoMode> ListAvailableVideoModes()
{
    std::vector<VideoMode> modes;

    DEVMODE dev;
    DWORD modeid = 0;

    while (EnumDisplaySettings(NULL, modeid++, &dev))
    {
        VideoMode mode(dev.dmPelsWidth, dev.dmPelsHeight);

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

bool PeekEvent(native_event_t& ev)
{
    MSG m;

    if (!PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
        return false;

    ev = native_event_t(m);

    return true;
}

void WaitEvent(native_event_t& ev)
{
    MSG m;

    GetMessage(&m, NULL, 0, 0);

    ev = native_event_t(m);
}

std::pair<bitflag<KeyModifier>, KeySymbol> translate_keydown_event(const native_event_t& key)
{
	const MSG& m = key;
    const uint_t native_keycode = (uint_t)m.wParam;

    std::pair<bitflag<KeyModifier>, KeySymbol> ret = {KeyModifier::None, KeySymbol::None};

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
            ret.first |= KeyModifier::Alt;
        if (native_keycode != VK_SHIFT && (shift_key & KEY_DOWN))
            ret.first |= KeyModifier::Shift;
        if (native_keycode != VK_CONTROL && (ctrl_key & KEY_DOWN))
            ret.first |= KeyModifier::Control;
        ret.second = (*it).wdk;
    }
    else if (native_keycode == VK_CONTROL)
    {
        if (GetKeyState(VK_LCONTROL) & KEY_DOWN)
            ret.second = KeySymbol::Control_L;
        else if (GetKeyState(VK_RCONTROL) & KEY_DOWN)
            ret.second = KeySymbol::Control_R;
    }
    else if (native_keycode == VK_MENU)
    {
        if (GetKeyState(VK_LMENU) & KEY_DOWN)
            ret.second = KeySymbol::Alt_L;
      /*  else if (GetAsyncKeyState(VK_RMENU) & KEY_DOWN)
            sym = keysym::alt_R;*/
    }
    else if (native_keycode == VK_SHIFT)
    {
       if (GetKeyState(VK_LSHIFT) & KEY_DOWN)
            ret.second = KeySymbol::Shift_L;
       else if (GetKeyState(VK_RSHIFT) & KEY_DOWN)
            ret.second = KeySymbol::Shift_R;
    }

    return ret;
}

std::pair<bitflag<KeyModifier>, MouseButton> translate_mouse_button_event(const native_event_t& btn)
{
    std::pair<bitflag<KeyModifier>, MouseButton> ret = {KeyModifier::None, MouseButton::None};    

    const auto alt   = GetKeyState(VK_MENU);
    const auto shift = GetKeyState(VK_SHIFT);
    const auto ctrl  = GetKeyState(VK_CONTROL);
    if (alt & KEY_DOWN)
        ret.first |= KeyModifier::Alt;
    if (shift & KEY_DOWN)
        ret.first |= KeyModifier::Shift;
    if (ctrl & KEY_DOWN)
        ret.first |= KeyModifier::Control;

    const auto wparam  = btn.get().wParam;
    const auto message = btn.get().message;
    if (message == WM_MOUSEWHEEL)
    {
        const auto hi = HIWORD(wparam);
        if (hi > 0)
            ret.second = MouseButton::WheelScrollUp;
        else ret.second = MouseButton::WheelScrollDown;
    }
    else 
    {
        switch (message)
        {
            case WM_LBUTTONUP:
            case WM_LBUTTONDOWN:
                ret.second = MouseButton::Left;
                break;
            case WM_MBUTTONUP:
            case WM_MBUTTONDOWN:
                ret.second = MouseButton::Wheel;
                break;
            case WM_RBUTTONUP:
            case WM_RBUTTONDOWN:
                ret.second = MouseButton::Right;
                break;
        }
        // other buttons as bitflags
        if (wparam & MK_LBUTTON)
            ret.second = MouseButton::Left;
        else if (wparam & MK_MBUTTON)
            ret.second = MouseButton::Wheel;
        else if (wparam & MK_RBUTTON)
            ret.second = MouseButton::Right;
    }
    return ret;
}

bool test_key_down(KeySymbol symbol)
{
    const UINT win    = find_keysym(symbol);
    const SHORT state = GetAsyncKeyState(win);

	return (state & KEY_DOWN) == KEY_DOWN;

}

bool test_key_down(uint_t keycode)
{
    assert(keycode);

    const SHORT state = GetAsyncKeyState(keycode);

	return (state & KEY_DOWN) == KEY_DOWN;
}

uint_t keysym_to_keycode(KeySymbol symbol)
{
    const UINT win = find_keysym(symbol);

    return win;
}

} // wdk
