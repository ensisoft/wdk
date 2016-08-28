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
#include <X11/Xutil.h>
#include <stdexcept>
#include <limits>
#include <cassert>
#include <cstring>
#include "../window_events.h"
#include "../window.h"
#include "../system.h"
#include "../videomode.h"
#include "../utf8.h"
#include "errorhandler.h"
#include "atoms.h"

namespace linux {
    long keysym2ucs(KeySym keysym);
}// linux

namespace wdk
{

struct window::impl {
    Window window;
    int width;
    int height;
    encoding enc;
    bool fullscreen;
};

window::window() : pimpl_(new impl)
{
    pimpl_->window = 0;
    pimpl_->enc    = encoding::utf8;
}

window::~window()
{
    if (exists())
        destroy();
}

void window::create(const std::string& title, uint_t width, uint_t height, uint_t visualid,
    bool can_resize, bool has_border, bool initially_visible)
{
    assert(width);
    assert(height);
    assert(!title.empty());
    assert(!pimpl_->window);

    Display* d = get_display_handle();

    int screen = DefaultScreen(d);
    int root   = RootWindow(d, screen);

    XVisualInfo vistemplate = {0};
    vistemplate.visualid    = visualid ? visualid : 0;
    const long visual_mask  = visualid ? VisualIDMask : 0;

    int num_visuals = 0;
    XVisualInfo* visinfo = XGetVisualInfo(d, visual_mask, &vistemplate, &num_visuals);
    if (!visinfo || !num_visuals)
        throw std::runtime_error("no such visual");

    XSetWindowAttributes attr;
    std::memset(&attr, 0, sizeof(attr));
    attr.colormap             = XCreateColormap(d, root, visinfo->visual, AllocNone);
    attr.event_mask           = KeyPressMask | KeyReleaseMask | // keyboard
                                ButtonPressMask | ButtonReleaseMask | // pointer aka. mouse clicked
                                EnterWindowMask | LeaveWindowMask   | // pointer aka. mouse leaves/enters window
                                PointerMotionMask | ButtonMotionMask | // pointer aka.mouse motion
                                StructureNotifyMask | // window size changed, mapping change (ConfigureNotify)
                                ExposureMask | // window exposure (paint)
                                FocusChangeMask; // lost, gain focus

    const unsigned long AttrMask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    factory<Window> win_factory(d);

    Window win = win_factory.create([&](Display* d)
    {
        Window ret = XCreateWindow(d,
            root,
            0, 0,
            width,
            height,
            0,
            visinfo->depth,
            InputOutput,
            visinfo->visual,
            AttrMask,
            &attr);
        return ret;
    });

    int visual_id = visinfo->visualid;

    XFree(visinfo);

    if (win_factory.has_error())
        throw std::runtime_error("failed to create window");

    XSetWMProtocols(d, win, &WM_DELETE_WINDOW, 1);
    XStoreName(d, win, title.c_str());

    if (!has_border)
    {
        struct hint {
            unsigned long flags;
            unsigned long functions;
            unsigned long decorations;
            long          inputMode;
            unsigned long status;
        };

        // get rid of window decorations
        hint hints = {0};
        hints.flags = 2;         // window decorations flag
        hints.decorations = 0;   // ...say bye bye
        XChangeProperty(d, win, _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32, PropModeReplace, static_cast<unsigned char*>((void*)&hints), 5);
    }

    if (!can_resize)
    {
        // make window unresizeable
        XSizeHints* hints = XAllocSizeHints();
        hints->flags      = PMinSize | PMaxSize;
        hints->min_width  = hints->max_width  = width;
        hints->min_height = hints->max_height = height;

        XSetWMSizeHints(d, win, hints, WM_SIZE_HINTS);
        XSetWMNormalHints(d, win, hints);
        XFree(hints);
    }

    if (initially_visible)
    {
        // show window
        XMapWindow(d, win);

        // X hack, since the api is asynchronous it's possible that
        // the client code can call a function such as set_setfocus which
        // fails siply because the WM hasn't mapped the window yet.
        // so we wait here  untill we're notified that it's actually mapped.
        XEvent ev;
        while (XCheckTypedWindowEvent(d, win, Expose, &ev) == False)
            (void)0;

        XPutBackEvent(d, &ev);
    }

    XFlush(d);

    pimpl_->window   = win;
    pimpl_->width    = 0;
    pimpl_->height   = 0;
    pimpl_->fullscreen = false;
}

void window::hide()
{
    assert(exists());

    Display* display = get_display_handle();

    XUnmapWindow(display, pimpl_->window);
}

void window::show()
{
    assert(exists());

    Display* display = get_display_handle();

    XMapWindow(display, pimpl_->window);
}

void window::destroy()
{
    assert(exists());

    Display* d = get_display_handle();

    if (pimpl_->fullscreen)
    {
        XUngrabPointer(d, CurrentTime);
        XUngrabKeyboard(d, CurrentTime);
    }

    XUnmapWindow(d, pimpl_->window);

    XDestroyWindow(d, pimpl_->window);

    XFlush(d);

    pimpl_->window = 0;

}

void window::invalidate()
{
    assert(exists());

    Display* d = get_display_handle();

    XClearArea(d, pimpl_->window,
        0, 0,
        surface_width(),
        surface_height(),
        True);
}

void window::move(int x, int y)
{
    assert(exists());
    assert(!is_fullscreen());

    Display* d = get_display_handle();

    XMoveWindow(d, pimpl_->window, x, y);

    XFlush(d);
}

void window::set_fullscreen(bool fullscreen)
{
    assert(exists());

    if (fullscreen == pimpl_->fullscreen)
        return;

    Display* d = get_display_handle();
    Window   w = handle();

    // todo: this is a bit slow at changing and will get confused if
    // multiple requests are made before the previous one is complete
    if (fullscreen)
    {
        XEvent ev;
        std::memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.xclient.message_type = _NET_WM_STATE;
        ev.xclient.format       = 32;
        ev.xclient.window       = w;
        ev.xclient.data.l[0]    = _NET_WM_STATE_ADD;
        ev.xclient.data.l[1]    = _NET_WM_STATE_FULLSCREEN;
        ev.xclient.data.l[3]    = w;
        XSendEvent(d, DefaultRootWindow(d), False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);

        // get exclusive keyboard access
        // note that this can fail if someone else has grabbed the keyboard already..
        XGrabKeyboard(d, w, True, GrabModeAsync, GrabModeAsync, CurrentTime);

        // get exclusive pointer (mouse) access
        XGrabPointer(d, w, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, w, None, CurrentTime);
    }
    else
    {
        XUngrabPointer(d, CurrentTime);

        XUngrabKeyboard(d, CurrentTime);

        XEvent ev;
        std::memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.xclient.message_type = _NET_WM_STATE;
        ev.xclient.format       = 32;
        ev.xclient.window       = w;
        ev.xclient.data.l[0]    = _NET_WM_STATE_REMOVE;
        ev.xclient.data.l[1]    = _NET_WM_STATE_FULLSCREEN;
        ev.xclient.data.l[3]    = w;
        XSendEvent(d, DefaultRootWindow(d), False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);

    }
    XFlush(d);

    pimpl_->fullscreen = fullscreen;
}

void window::set_focus()
{
    assert(exists());

    Display* d = get_display_handle();

    XRaiseWindow(d, pimpl_->window);
    XSetInputFocus(d, pimpl_->window, RevertToNone, CurrentTime);

    XFlush(d);
}

void window::set_size(uint_t width, uint_t height)
{
    assert(width);
    assert(height);
    assert(exists());
    assert(!is_fullscreen());

    Display* d = get_display_handle();

    XResizeWindow(d, pimpl_->window, width, height);

    XFlush(d);
}

void window::set_encoding(encoding enc)
{
    pimpl_->enc = enc;
}

bool window::process_event(const native_event_t& ev)
{
    if (ev.get_window_handle() != handle())
        return false;

    const XEvent& event = ev;

    switch (event.type)
    {
        case MotionNotify:
            if (on_mouse_move)
            {
                window_event_mouse_move mickey = {};
                mickey.window_x = event.xmotion.x;
                mickey.window_y = event.xmotion.y;
                mickey.global_x = event.xmotion.x_root;
                mickey.global_y = event.xmotion.y_root;
                on_mouse_move(mickey);
            }
            break;

        case ButtonPress:
            if (on_mouse_press)
            {
                const auto& button = translate_mouse_button_event(ev);

                window_event_mouse_press mickey = {};
                mickey.window_x = event.xbutton.x;
                mickey.window_y = event.xbutton.y;
                mickey.global_x = event.xbutton.x_root;
                mickey.global_y = event.xbutton.y_root;
                mickey.modifiers = button.first;
                mickey.btn       = button.second;
                on_mouse_press(mickey);
            }
            break;

        case ButtonRelease:
            if (on_mouse_release)
            {
                const auto& button = translate_mouse_button_event(ev);

                window_event_mouse_release mickey = {};
                mickey.window_x = event.xbutton.x;
                mickey.window_y = event.xbutton.y;
                mickey.global_x = event.xbutton.x_root;
                mickey.global_y = event.xbutton.y_root;
                mickey.modifiers = button.first;
                mickey.btn       = button.second;
                on_mouse_release(mickey);
            }
            break;

        case FocusIn:
            if (on_gain_focus)
                on_gain_focus(window_event_focus{});
            break;

        case FocusOut:
            if (on_lost_focus)
                on_lost_focus(window_event_focus{});
            break;

        case Expose:
            if (on_paint)
            {
                window_event_paint paint = {0};
                paint.x      = event.xexpose.x;
                paint.y      = event.xexpose.y;
                paint.width  = event.xexpose.width;
                paint.height = event.xexpose.height;
                on_paint(paint);
            }
            break;

        case ConfigureNotify:
            if (pimpl_->width != event.xconfigure.width || pimpl_->height != event.xconfigure.height)
            {
                pimpl_->width  = event.xconfigure.width;
                pimpl_->height = event.xconfigure.height;
                if (on_resize)
                {
                    window_event_resize resize = {0};
                    resize.width  = event.xconfigure.width;
                    resize.height = event.xconfigure.height;
                    on_resize(resize);
                }
            }
            break;

        case CreateNotify:
            if (on_create)
            {
                window_event_create create = {0};
                create.x      = event.xcreatewindow.x;
                create.y      = event.xcreatewindow.y;
                create.width  = event.xcreatewindow.width;
                create.height = event.xcreatewindow.height;
                on_create(create);
            }
            break;


        case ClientMessage:
            if ((Atom)event.xclient.data.l[0] == WM_DELETE_WINDOW)
            {
                if (on_want_close)
                    on_want_close(window_event_want_close{});
            }
            break;

        case KeyPress:
            if (on_keydown)
            {
                const auto& keys = translate_keydown_event(ev);
                if (keys.second != keysym::none)
                    on_keydown(window_event_keydown{keys.second, keys.first});
            }
            if (on_char)
            {
                KeySym sym = NoSymbol;
                XLookupString(const_cast<XKeyEvent*>(&event.xkey), nullptr, 0, &sym, nullptr);
                if (sym == NoSymbol)
                    break;

                const long ucs2 = linux::keysym2ucs(sym);
                if (ucs2 == -1)
                    break;

                // simulate WM_CHAR and synthesize a character event
                // note that ClientMessage didn't work as expected.
                static_assert(sizeof(Window) >= sizeof(ucs2), "");

                XEvent hack;
                std::memset(&hack, 0, sizeof(hack));
                hack.type        = MapNotify;
                hack.xmap.event  = ucs2;
                hack.xmap.window = handle();
                XSendEvent(event.xany.display, handle(), False, 0, &hack);
            }
            break;

        case KeyRelease:
            if (on_keyup)
            {
                const auto& keys = translate_keydown_event(ev);
                if (keys.second != keysym::none)
                    on_keyup(window_event_keyup{keys.second, keys.first});
            }
            break;

        case MapNotify:
            if (!event.xany.send_event)
                break;

            if (on_char)
            {
                const XMapEvent& uchar = event.xmap;
                const long ucs2 = (long)uchar.event;

                window_event_char c = {0};

                if (pimpl_->enc == encoding::ascii)
                    c.ascii = ucs2 & 0x7f;
                else if (pimpl_->enc == encoding::ucs2)
                    c.ucs2 = ucs2;
                else if (pimpl_->enc == encoding::utf8)
                    enc::utf8_encode(&ucs2, &ucs2 + 1, &c.utf8[0]);

                on_char(c);
            }
            break;

    }
    return true;
}

uint_t window::surface_width() const
{
    assert(exists());

    Display* d = get_display_handle();

    XWindowAttributes attrs;
    XGetWindowAttributes(d, pimpl_->window, &attrs);

    return attrs.width;
}

uint_t window::surface_height() const
{
    assert(exists());

    Display* d = get_display_handle();

    XWindowAttributes attrs;
    XGetWindowAttributes(d, pimpl_->window, &attrs);

    return attrs.height;
}

bool window::exists() const
{
    return pimpl_->window != 0;
}

bool window::is_fullscreen() const
{
    return pimpl_->fullscreen;
}


window::encoding window::get_encoding() const
{
    return pimpl_->enc;
}

native_window_t window::handle() const
{
    return native_window_t {pimpl_->window};
}

std::pair<uint_t, uint_t> window::min_size() const
{
    assert(exists());

    Display* d = get_display_handle();

    XSizeHints* hints = XAllocSizeHints();

    long supplied = 0;
    XGetWMNormalHints(d, pimpl_->window, hints, &supplied);

    // there aren't any hints unless the application(?)
    // has set them. (xprop, is a good tool to check).
    // however if we try to XResizeWindow to 0,0 it will generate BadValue
    // so we clamp to 1 minimum

    if (hints->min_width == 0)
        hints->min_width = 1;
    if (hints->min_height  == 0)
        hints->min_height = 1;

    const std::pair<uint_t, uint_t> ret {hints->min_width, hints->min_height};

    XFree(hints);

    return ret;

}

std::pair<uint_t, uint_t> window::max_size() const
{
    assert(exists());

    Display* d = get_display_handle();

    XSizeHints* hints = XAllocSizeHints();

    long supplied = 0;
    XGetWMNormalHints(d, pimpl_->window, hints, &supplied);

    if (hints->max_width == 0)
        hints->max_width = std::numeric_limits<uint16_t>::max();
    if (hints->max_height == 0)
        hints->max_height = std::numeric_limits<uint16_t>::max();

    const std::pair<uint_t, uint_t> ret {hints->max_width, hints->max_height};

    XFree(hints);

    return ret;
}
} // wdk

