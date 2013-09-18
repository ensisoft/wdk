
#include <windows.h>
#include <algorithm>
#include <iterator>
#include <cassert>
#include "../keyboard.h"
#include "../events.h"
#include "../event.h"

namespace {
    using namespace wdk;

    struct key_mapping {
        keysym sym;
        UINT native;
    };
    bool operator<(const key_mapping& i, const key_mapping& j)
    {
        return i.native < j.native;
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
        {keysym::alt_R,         VK_RMENU},
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

    std::string get_vk_name(UINT vk)
    {
        const UINT scancode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
        std::string str;
        str.resize(50);
        const int ret = GetKeyNameText(scancode << 16, &str[0], str.size());
        str.resize(ret);
        return str;
    }
} // namespace

namespace wdk
{

struct keyboard::impl {};

keyboard::keyboard(native_display_t disp) 
{
    // sort the table by VK values for quicker lookup win32->wdk
    std::sort(std::begin(keymap), std::end(keymap));
}

keyboard::~keyboard()
{
}

std::string keyboard::name(wdk::keymod modifier)
{
    switch (modifier)
    {
        case keymod::none:    return "None";
        case keymod::shift:   return "Shift";
        case keymod::control: return "Ctrl";
        case keymod::alt:     return "Alt";
        case keymod::super:   return "Super";
        case keymod::hyper:   return "Hyper";
        default:
        assert(0);
        break;
    }
    return "";
}

std::string keyboard::name(wdk::keysym symbol)
{
    auto it = std::find_if(std::begin(keymap), std::end(keymap), 
        [=] (const key_mapping& map)
        {
            return map.sym == symbol;
        });

    if (it == std::end(keymap))
        return "";

    return get_vk_name((*it).native);
}

std::string keyboard::name(uint_t native_keycode)
{
    return get_vk_name(native_keycode);
}

std::pair<keymod, keysym> keyboard::translate(event& ev)
{
    const MSG& msg = ev.ev;
    if (msg.message != WM_KEYUP && msg.message != WM_KEYDOWN)
        return std::make_pair(keymod::none, keysym::none);

    const UINT vk  = ev.ev.wParam;
    const UINT mod = 0; 

    return translate(mod, vk);
}

std::pair<keymod, keysym> keyboard::translate(uint_t native_modifier, uint_t native_keycode)
{
    std::pair<keymod, keysym> ret(keymod::none, keysym::none);

    const key_mapping needle = {keysym::none, native_keycode};

    enum { KEY_DOWN = 0x8000 };

    const auto it = std::lower_bound(std::begin(keymap), std::end(keymap), needle);
    if (it != std::end(keymap) && (*it).native == native_keycode)
    {
        keymod mods = keymod::none;
        const short alt_key   = GetKeyState(VK_MENU);
        const short shift_key = GetKeyState(VK_SHIFT);
        const short ctrl_key  = GetKeyState(VK_CONTROL);

        if (native_keycode != VK_MENU && (alt_key & KEY_DOWN))
            mods |= keymod::alt;
        if (native_keycode != VK_SHIFT && (shift_key & KEY_DOWN))
            mods |= keymod::shift;
        if (native_keycode != VK_CONTROL && (ctrl_key & KEY_DOWN))
            mods |= keymod::control;

        ret.first = mods;
        ret.second = (*it).sym;
    }
    else if (native_keycode == VK_CONTROL)
    {
        if (GetAsyncKeyState(VK_LCONTROL) & KEY_DOWN)
            ret.second = keysym::control_L;
        else if (GetAsyncKeyState(VK_RCONTROL) & KEY_DOWN)
            ret.second = keysym::control_R;
    }
    else if (native_keycode == VK_MENU)
    {
        if (GetAsyncKeyState(VK_LMENU) & KEY_DOWN)
            ret.second = keysym::alt_L;
        else if (GetAsyncKeyState(VK_RMENU) & KEY_DOWN)
            ret.second = keysym::alt_R;
    }
    else if (native_keycode == VK_SHIFT)
    {
        if (GetAsyncKeyState(VK_LSHIFT) & KEY_DOWN)
            ret.second = keysym::shift_L;
        else if (GetAsyncKeyState(VK_RSHIFT) & KEY_DOWN)
            ret.second = keysym::shift_R;
    }
    return ret;
}

bool keyboard::dispatch_event(event& ev)
{
    // translate virtual key messages into unicode characters 
    // which are posted in WM_CHAR (UTF-16)
    // this works fine for BMP in the range 0x0000 - 0xD7FF, 0xE000 - 0xFFFF.
    // for values above 0xFFFF we should decode the UTF-16 to a single code point value
    // in order to be consistent with Linux implementation.
    TranslateMessage(&ev.ev); 

    if (ev.type == event_type::keyboard_char && event_char)
    {
        const MSG& msg = ev.ev;
        keyboard_event_char ch = {msg.wParam};
        event_char(ch);
        return true;
    }
    else if (ev.type == event_type::keyboard_keyup && event_keyup)
    {
        const auto key = translate(ev);
        if (key.second != keysym::none)
        {
            keyboard_event_keyup up = {key.second, key.first};
            event_keyup(up);
        }
        return true;
    }
    else if (ev.type == event_type::keyboard_keydown && event_keydown)
    {
        const auto key = translate(ev);
        if (key.second != keysym::none)
        {
            keyboard_event_keydown down = {key.second, key.first};
            event_keydown(down);
        }
        return true;
    } 

    return false;
}

} // wdk
