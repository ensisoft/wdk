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
        keysym  wdk;
        KeySym  x11;
    };
    bool operator<(const key_mapping& i, const key_mapping& j)
    {
        return i.wdk < j.wdk;
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
        //{keysym::alt_R,                 XK_Alt_R},
        {keysym::capslock,              XK_Caps_Lock},
        {keysym::shift_L,               XK_Shift_L},
        {keysym::shift_R,               XK_Shift_R},
        {keysym::insert,                XK_Insert},
        {keysym::del,                   XK_Delete},
        {keysym::home,                  XK_Home},
        {keysym::end,                   XK_End},
        {keysym::pageup,                XK_Page_Up},
        {keysym::pagedown,              XK_Page_Down},
        {keysym::left,                  XK_Left},
        {keysym::right,                 XK_Right},
        {keysym::down,                  XK_Down},
        {keysym::up,                    XK_Up},
        {keysym::escape,                XK_Escape}
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

    KeySym find_keysym(keysym sym)
    {
        assert(sym != keysym::none);

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

const long _NET_WM_STATE_REMOVE = 0;
const long _NET_WM_STATE_ADD    = 1;
const long _NET_WM_STATE_TOGGLE = 2;

int AltMask;
int XRandREventBase;

native_display_t get_display_handle()
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

videomode get_current_video_mode()
{
    Display* dpy = get_display_handle();
    int root = RootWindow(dpy, DefaultScreen(dpy));

    auto config = make_unique_ptr(XRRGetScreenInfo(dpy, root), XRRFreeScreenConfigInfo);
    if (!config)
        throw std::runtime_error("Xrandr get config failed");

    Rotation rot;
    const int cur_mode_index = XRRConfigCurrentConfiguration(config.get(), &rot);

    int size_count;
    XRRScreenSize* sizes = XRRConfigSizes(config.get(), &size_count);

    videomode vm;
    vm.xres = sizes[cur_mode_index].width;
    vm.yres = sizes[cur_mode_index].height;
    return vm;
}

void set_video_mode(const videomode& m)
{
    Display* dpy  = get_display_handle();
    int root = RootWindow(dpy, DefaultScreen(dpy));

    auto config = make_unique_ptr(XRRGetScreenInfo(dpy, root), XRRFreeScreenConfigInfo);
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

std::vector<videomode> list_video_modes()
{
    std::vector<videomode> modes;

    Display* dpy  = get_display_handle();
    int root = RootWindow(dpy, DefaultScreen(dpy));

    auto config = make_unique_ptr(XRRGetScreenInfo(dpy, root), XRRFreeScreenConfigInfo);
    if (!config)
        throw std::runtime_error("Xrandrd get config failed");

    int size_count;
    XRRScreenSize* sizes = XRRConfigSizes(config.get(), &size_count);

    for (int i=0; i<size_count; ++i)
    {
        videomode vm;
        vm.xres = sizes[i].width;
        vm.yres = sizes[i].height;
        modes.push_back(vm);
    }
    return modes;

}

bool have_events()
{
    // XPending flushes the output queue
    return (XPending(get_display_handle()) != 0);
}

bool sync_events()
{
    // X11 is an async protocol with cached state on the client side inside Xlib.
    // hence we have the following problem that the following code can fail surprisingly.
    // window w; ... w.set_size(x, y); assert(w.surface_width() == x)
    // the reason why it might fail is that set_size will generate a request to the X server
    // which then processes the change and sends back the response. Xlib will contain
    // cached state for the window size untill it receives a response from the server
    // for the size request.
    // so this sync function tries to make sure that the event queue is fully processed and
    // the Xlib state is synced with the latest state changes originating from the client.

    Display* d = get_display_handle();

    unsigned long next = XNextRequest(d);
    if (last_event_received >= next -1)
        return false;

    XSync(d, False);

    std::vector<XEvent> events;

    while (true)
    {
        XEvent x;
        XNextEvent(d, &x);

        events.push_back(x);

        // assuming a reasonable sequential operation here.
        if (x.xany.serial >= next - 1)
            break;
    }

    // in order not to mess with the actual event signaling semantics
    // in other code, we put the events back into the queue in the same
    // order that the're supposed to be there. (XPutBack puts in the front of the queue!)
    for (auto it = events.rbegin(); it != events.rend(); ++it)
        XPutBackEvent(d, &(*it));

    // ho hum.. still doesn't work without this little hack here.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return true;
}

native_event_t get_event()
{
    XEvent event = {0};

    // XNextEvent flushes the output queue
    XNextEvent(get_display_handle(), &event);

    last_event_received = event.xany.serial;

    XRRUpdateConfiguration(&event);

    return native_event_t(event);
}

bool peek_event(native_event_t& ev)
{
    if (!XPending(get_display_handle()))
        return false;

    XEvent event = {0};

    // XPeekEvent flushes the output queue
    XPeekEvent(get_display_handle(), &event);

    last_event_received = event.xany.serial;

    // update Xlib state when XrandR events are received
    XRRUpdateConfiguration(&event);

    ev = native_event_t(event);

    return true;
}

std::pair<bitflag<keymod>, keysym> translate_keydown_event(const native_event_t& key)
{
    std::pair<bitflag<keymod>, keysym> ret = {keymod::none, keysym::none};

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
        ret.first |= keymod::alt;
    if (native_modifier & ControlMask)
        ret.first |= keymod::control;
    if (native_modifier & ShiftMask)
        ret.first |= keymod::shift;

    return ret;
}

std::pair<bitflag<keymod>, button> translate_mouse_button_event(const native_event_t& btn)
{
    button b = button::none;
    bitflag<keymod> m { keymod::none };

    const auto button = btn.get().xbutton.button;

    if (button == Button1)
        b = wdk::button::left;
    else if (button == Button2)
        b = wdk::button::wheel;
    else if (button == Button3)
        b = wdk::button::right;
    else if (button == Button4)
        b = wdk::button::wheel_up;
    else if (button == Button5)
        b = wdk::button::wheel_down;
    else if (button == Button5 + 1)
        b = wdk::button::thumb1;
    else if (button == Button5 + 2)
        b = wdk::button::thumb2;
    else if (button == Button5 + 3)
        b = wdk::button::thumb3;
    else if (button == Button5 + 4)
        b = wdk::button::thumb4;

    // todo: thumb1 and thumb2 buttons
    const auto state  = btn.get().xbutton.state;

    if (state & AltMask)
        m.set(keymod::alt);
    if (state & ControlMask)
        m.set(keymod::control);
    if (state & ShiftMask)
        m.set(keymod::shift);

    return { m, b };
}

bool test_key_down(keysym symbol)
{
    const KeySym sym = find_keysym(symbol);
    const KeyCode code = XKeysymToKeycode(get_display_handle(), sym);

    return test_key_down(code);
}

bool test_key_down(uint_t keycode)
{
    assert(keycode);

    Display* d = get_display_handle();

    uint8_t key_states[32];
    XQueryKeymap(d, (char*)key_states);

    return bool(key_states[keycode / 8] & (1 << (keycode % 8)));
}

uint_t keysym_to_keycode(keysym symbol)
{
    Display* d = get_display_handle();

    const KeySym sym = find_keysym(symbol);
    const KeyCode code = XKeysymToKeycode(d, sym);

    return uint_t(code);
}

} // wdk
