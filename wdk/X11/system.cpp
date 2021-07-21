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

// we're getting a warning about XKeycodeToKeysym. Suppress this for now.
#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUG__)
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <sys/select.h>
#include <stdexcept>
#include <algorithm>
#include <stack>
#include <chrono>
#include <thread>
#include <cassert>
#include "../system.h"
#include "../utility.h"
#include "../videomode.h"
#include "../keys.h"

namespace {

    using namespace wdk;

    struct key_mapping {
        Keysym  wdk;
        KeySym  x11;
    };
    bool operator<(const key_mapping& i, const key_mapping& j)
    {
        return i.wdk < j.wdk;
    }

    static key_mapping keymap[] = {
        {Keysym::None,                 NoSymbol},
        {Keysym::Backspace,            XK_BackSpace},
        {Keysym::Tab,                  XK_Tab},
        {Keysym::Enter,                XK_Return},
        {Keysym::Space,                XK_space},
        {Keysym::Key0,                 XK_0},
        {Keysym::Key1,                 XK_1},
        {Keysym::Key2,                 XK_2},
        {Keysym::Key3,                 XK_3},
        {Keysym::Key4,                 XK_4},
        {Keysym::Key5,                 XK_5},
        {Keysym::Key6,                 XK_6},
        {Keysym::Key7,                 XK_7},
        {Keysym::Key8,                 XK_8},
        {Keysym::Key9,                 XK_9},
        {Keysym::KeyA,                 XK_a},
        {Keysym::KeyB,                 XK_b},
        {Keysym::KeyC,                 XK_c},
        {Keysym::KeyD,                 XK_d},
        {Keysym::KeyE,                 XK_e},
        {Keysym::KeyF,                 XK_f},
        {Keysym::KeyG,                 XK_g},
        {Keysym::KeyH,                 XK_h},
        {Keysym::KeyI,                 XK_i},
        {Keysym::KeyJ,                 XK_j},
        {Keysym::KeyK,                 XK_k},
        {Keysym::KeyL,                 XK_l},
        {Keysym::KeyM,                 XK_m},
        {Keysym::KeyN,                 XK_n},
        {Keysym::KeyO,                 XK_o},
        {Keysym::KeyP,                 XK_p},
        {Keysym::KeyQ,                 XK_q},
        {Keysym::KeyR,                 XK_r},
        {Keysym::KeyS,                 XK_s},
        {Keysym::KeyT,                 XK_t},
        {Keysym::KeyU,                 XK_u},
        {Keysym::KeyV,                 XK_v},
        {Keysym::KeyW,                 XK_w},
        {Keysym::KeyX,                 XK_x},
        {Keysym::KeyY,                 XK_y},
        {Keysym::KeyZ,                 XK_z},
        {Keysym::KeyA,                 XK_A},
        {Keysym::KeyB,                 XK_B},
        {Keysym::KeyC,                 XK_C},
        {Keysym::KeyD,                 XK_D},
        {Keysym::KeyE,                 XK_E},
        {Keysym::KeyF,                 XK_F},
        {Keysym::KeyG,                 XK_G},
        {Keysym::KeyH,                 XK_H},
        {Keysym::KeyI,                 XK_I},
        {Keysym::KeyJ,                 XK_J},
        {Keysym::KeyK,                 XK_K},
        {Keysym::KeyL,                 XK_L},
        {Keysym::KeyM,                 XK_M},
        {Keysym::KeyN,                 XK_N},
        {Keysym::KeyO,                 XK_O},
        {Keysym::KeyP,                 XK_P},
        {Keysym::KeyQ,                 XK_Q},
        {Keysym::KeyR,                 XK_R},
        {Keysym::KeyS,                 XK_S},
        {Keysym::KeyT,                 XK_T},
        {Keysym::KeyU,                 XK_U},
        {Keysym::KeyV,                 XK_V},
        {Keysym::KeyW,                 XK_W},
        {Keysym::KeyX,                 XK_X},
        {Keysym::KeyY,                 XK_Y},
        {Keysym::KeyZ,                 XK_Z},
        {Keysym::F1,                   XK_F1},
        {Keysym::F2,                   XK_F2},
        {Keysym::F3,                   XK_F3},
        {Keysym::F4,                   XK_F4},
        {Keysym::F5,                   XK_F5},
        {Keysym::F6,                   XK_F6},
        {Keysym::F7,                   XK_F7},
        {Keysym::F8,                   XK_F8},
        {Keysym::F9,                   XK_F9},
        {Keysym::F10,                  XK_F10},
        {Keysym::F11,                  XK_F11},
        {Keysym::F12,                  XK_F12},
        {Keysym::ControlL,             XK_Control_L},
        {Keysym::ControlR,             XK_Control_R},
        {Keysym::AltL,                 XK_Alt_L},
        //{keysym::alt_R,              XK_Alt_R},
        {Keysym::CapsLock,             XK_Caps_Lock},
        {Keysym::ShiftL,               XK_Shift_L},
        {Keysym::ShiftR,               XK_Shift_R},
        {Keysym::Insert,               XK_Insert},
        {Keysym::Del,                  XK_Delete},
        {Keysym::Home,                 XK_Home},
        {Keysym::End,                  XK_End},
        {Keysym::PageUp,               XK_Page_Up},
        {Keysym::PageDown,             XK_Page_Down},
        {Keysym::ArrowLeft,            XK_Left},
        {Keysym::ArrowRight,           XK_Right},
        {Keysym::ArrowDown,            XK_Down},
        {Keysym::ArrowUp,              XK_Up},
        {Keysym::Escape,               XK_Escape},
        {Keysym::Plus,                 XK_plus},
        {Keysym::Minus,                XK_minus}
        // why does Right ALT Generate XK_ISO_Level3_Shift
        // instead of XK_Alt_R ?? -> XCK_ISO_Level3_Shift = AltGR
        //{keysym::alt_R,                 XK_ISO_Level3_Shift}
    };

    struct table_sorter {
        table_sorter()
        {
            // sort the keymap table for binary lookup on wdk::keysym
            std::sort(std::begin(keymap), std::end(keymap));
        }
    } sort_my_data_bitch;

    KeySym find_keysym(Keysym sym)
    {
        assert(sym != Keysym::None);

        // binary search the table
        const auto it = std::lower_bound(std::begin(keymap), std::end(keymap), key_mapping{sym, 0});

        // should be there.
        assert(it != std::end(keymap));

        return (*it).x11;
    }


unsigned long last_event_received;

} // namespace

namespace wdk
{

Atom _NET_WM_STATE;
Atom _NET_WM_STATE_FULLSCREEN;
Atom _MOTIF_WM_HINTS;

Atom WM_SIZE_HINTS;
Atom WM_DELETE_WINDOW;

// see comments about the constness in the atoms.h
long _NET_WM_STATE_REMOVE = 0;
long _NET_WM_STATE_ADD    = 1;
long _NET_WM_STATE_TOGGLE = 2;


int AltMask;
int XRandREventBase;

native_display_t GetNativeDisplayHandle()
{
    struct open_display {
        Display* d;

        open_display() : d(XOpenDisplay(nullptr))
        {
            if (!d)
                throw std::runtime_error("cannot open X display");

            int major, minor;
            if (!XRRQueryVersion(d, &major, &minor))
                throw std::runtime_error("XRandR is not available");

            // get event base
            int event_base = 0;
            int error_base = 0;
            XRRQueryExtension(d, &event_base, &error_base);

            XRandREventBase = event_base;

            int root = RootWindow(d, DefaultScreen(d));
            // set masks on our root window handle.
            // StructureNotifyMask means that we receive events that pertain to
            // root window's structure changes such as ConfigureNotify.
            // SubstructureNotifyMask means that we get events that pertain to
            // child windows of the root window, i.e. create/destroy events.
            // (our windows are children of the root)
            XSelectInput(d, root, StructureNotifyMask | SubstructureNotifyMask);

            // set input mask to get XRandR notifications
            XRRSelectInput(d, root, RRScreenChangeNotifyMask);

            // get the modifier map for finding XK_Alt_L or XK_Alt_R
            XModifierKeymap* mods = XGetModifierMapping(d);

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
                        sym = XKeycodeToKeysym(d, code, group++);
                    }
                    while (sym == NoSymbol && group < 4);

                    if (sym == XK_Alt_L)
                    {
                        AltMask |= (1 << mod);
                        //printf("Found LeftAlt mask %x", (1<<mod));
                    }
                    else if (sym == XK_Alt_R)
                    {
                        AltMask |= (1 << mod);
                       //printf("Found RightAlt mask %x", (1<<mod));
                    }
                }
            }
            XFreeModifiermap(mods);

            WM_DELETE_WINDOW = XInternAtom(d, "WM_DELETE_WINDOW", True);
            WM_SIZE_HINTS    = XInternAtom(d, "WM_SIZE_HINTS", True);
            _MOTIF_WM_HINTS  = XInternAtom(d, "_MOTIF_WM_HINTS", True);
            _NET_WM_STATE    = XInternAtom(d, "_NET_WM_STATE", True);
            _NET_WM_STATE_FULLSCREEN = XInternAtom(d, "_NET_WM_STATE_FULLSCREEN", True);
        }
       ~open_display()
        {
            XCloseDisplay(d);
        }
    };
    static open_display dpy;

    return dpy.d;
}

VideoMode GetCurrentVideoMode()
{
    Display* dpy = GetNativeDisplayHandle();
    int root = RootWindow(dpy, DefaultScreen(dpy));

    auto config = MakeUniqueHandle(XRRGetScreenInfo(dpy, root), XRRFreeScreenConfigInfo);
    if (!config)
        throw std::runtime_error("Xrandr get config failed");

    Rotation rot;
    const int cur_mode_index = XRRConfigCurrentConfiguration(config.get(), &rot);

    int size_count;
    XRRScreenSize* sizes = XRRConfigSizes(config.get(), &size_count);

    VideoMode vm;
    vm.xres = sizes[cur_mode_index].width;
    vm.yres = sizes[cur_mode_index].height;
    return vm;
}

void SetVideoMode(const VideoMode& m)
{
    Display* dpy  = GetNativeDisplayHandle();
    int root = RootWindow(dpy, DefaultScreen(dpy));

    auto config = MakeUniqueHandle(XRRGetScreenInfo(dpy, root), XRRFreeScreenConfigInfo);
    if (!config)
        throw std::runtime_error("Xrandr get config failed");

    int size_count;
    XRRScreenSize* sizes = XRRConfigSizes(config.get(), &size_count);

    int found_index = -1;
    for (int i=0; i<size_count; ++i)
    {
        if (sizes[i].width == (int)m.xres && sizes[i].height == (int)m.yres)
        {
            found_index = i;
            break;
        }
    }
    if (found_index == -1)
        throw std::runtime_error("invalid video mode");

    Rotation rot;
    const int cur_mode_index = XRRConfigCurrentConfiguration(config.get(), &rot);
    if (found_index == cur_mode_index)
        return;

    if (XRRSetScreenConfig(dpy, config.get(), root, found_index, rot, CurrentTime) == RRSetConfigFailed)
        throw std::runtime_error("Xrandr set video mode failed");
}

std::vector<VideoMode> ListVideoModes()
{
    std::vector<VideoMode> modes;

    Display* dpy  = GetNativeDisplayHandle();
    int root = RootWindow(dpy, DefaultScreen(dpy));

    auto config = MakeUniqueHandle(XRRGetScreenInfo(dpy, root), XRRFreeScreenConfigInfo);
    if (!config)
        throw std::runtime_error("Xrandrd get config failed");

    int size_count;
    XRRScreenSize* sizes = XRRConfigSizes(config.get(), &size_count);

    for (int i=0; i<size_count; ++i)
    {
        VideoMode vm;
        vm.xres = sizes[i].width;
        vm.yres = sizes[i].height;
        modes.push_back(vm);
    }
    return modes;

}

bool PeekEvent(native_event_t& ev)
{
    Display* d = GetNativeDisplayHandle();

    if (!XPending(d))
        return false;

    XEvent event = {0};
    XNextEvent(d, &event);

    // Update Xlib state when XrandR events are received.
    XRRUpdateConfiguration(&event);

    ev = native_event_t(event);
    return true;
}

void WaitEvent(native_event_t& ev)
{
    Display* d = GetNativeDisplayHandle();

    XEvent event = {0};
    XNextEvent(d, &event);

    // Update xlib state when XrandR events are received.
    XRRUpdateConfiguration(&event);

    ev = native_event_t(event);
}


std::pair<bitflag<Keymod>, Keysym> TranslateKeydownEvent(const native_event_t& key)
{
    std::pair<bitflag<Keymod>, Keysym> ret = {Keymod::None, Keysym::None};

    const XEvent& ev = key;

    XKeyEvent xev = ev.xkey;

    // disregard modifiers for symbol lookup, only want to look up the keysym,
    // not X's idea of translated keysym+modifier
    xev.state = 0;

    KeySym sym = NoSymbol;
    XLookupString(&xev, NULL, 0, &sym, NULL);
    if (sym == NoSymbol)
        return ret;

    // have to do a linear search since the keyboard mapping table
    // is sorted by X11 values.
    const auto it = std::find_if(std::begin(keymap), std::end(keymap),
        [=] (const key_mapping& map)
        {
            return map.x11 == sym;
        });
    if (it == std::end(keymap))
        return ret;

    const uint native_modifier = ev.xkey.state;

    ret.second = (*it).wdk;
    if (native_modifier & AltMask)
        ret.first |= Keymod::Alt;
    if (native_modifier & ControlMask)
        ret.first |= Keymod::Control;
    if (native_modifier & ShiftMask)
        ret.first |= Keymod::Shift;

    return ret;
}

std::pair<bitflag<Keymod>, MouseButton> TranslateMouseButtonEvent(const native_event_t& btn)
{
    MouseButton b = MouseButton::None;
    bitflag<Keymod> m { Keymod::None };

    const auto button = btn.get().xbutton.button;

    if (button == Button1)
        b = wdk::MouseButton::Left;
    else if (button == Button2)
        b = wdk::MouseButton::Wheel;
    else if (button == Button3)
        b = wdk::MouseButton::Right;
    else if (button == Button4)
        b = wdk::MouseButton::WheelScrollUp;
    else if (button == Button5)
        b = wdk::MouseButton::WheelScrollDown;
    else if (button == Button5 + 1)
        b = wdk::MouseButton::Thumb1;
    else if (button == Button5 + 2)
        b = wdk::MouseButton::Thumb2;
    else if (button == Button5 + 3)
        b = wdk::MouseButton::Thumb3;
    else if (button == Button5 + 4)
        b = wdk::MouseButton::Thumb4;

    // todo: thumb1 and thumb2 buttons
    const auto state  = btn.get().xbutton.state;

    if (state & AltMask)
        m.set(Keymod::Alt);
    if (state & ControlMask)
        m.set(Keymod::Control);
    if (state & ShiftMask)
        m.set(Keymod::Shift);

    return { m, b };
}

bool TestKeyDown(Keysym symbol)
{
    const KeySym sym = find_keysym(symbol);
    const KeyCode code = XKeysymToKeycode(GetNativeDisplayHandle(), sym);

    return TestKeyDown(code);
}

bool TestKeyDown(uint_t keycode)
{
    assert(keycode);

    Display* d = GetNativeDisplayHandle();

    uint8_t key_states[32];
    XQueryKeymap(d, (char*)key_states);

    return bool(key_states[keycode / 8] & (1 << (keycode % 8)));
}

uint_t MapKeysymToNativeKeycode(Keysym symbol)
{
    Display* d = GetNativeDisplayHandle();

    const KeySym sym = find_keysym(symbol);
    const KeyCode code = XKeysymToKeycode(d, sym);

    return uint_t(code);
}

} // wdk
