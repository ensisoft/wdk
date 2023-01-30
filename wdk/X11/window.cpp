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

#include "wdk/events.h"
#include "wdk/window.h"
#include "wdk/system.h"
#include "wdk/videomode.h"
#include "wdk/utf8.h"
#include "wdk/X11/errorhandler.h"
#include "wdk/X11/atoms.h"

#define X11_None 0L
#define X11_RevertToNone 0

// g++ -std=gnu++14 defines linux (doh)
#undef linux

namespace linux {
    long keysym2ucs(KeySym keysym);
}// linux

namespace wdk
{

struct Window::impl {
    ::Window window = 0;
    int width = 0;
    int height = 0;
    Encoding enc;
    bool fullscreen = false;
    bool cursor     = true;
    bool mouse_grab = false;
};

Window::Window() : pimpl_(new impl)
{
    pimpl_->window = 0;
    pimpl_->enc    = Encoding::UTF8;
}

Window::~Window()
{
    if (DoesExist())
        Destroy();
}

void Window::Create(const std::string& title, uint_t width, uint_t height, uint_t visualid,
    bool can_resize, bool has_border, bool initially_visible)
{
    assert(width);
    assert(height);
    assert(!title.empty());
    assert(!pimpl_->window);

    Display* d = GetNativeDisplayHandle();

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

    factory<::Window> win_factory(d);

    ::Window win = win_factory.create([&](Display* d)
    {
        ::Window ret = XCreateWindow(d,
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

void Window::Hide()
{
    assert(DoesExist());

    Display* display = GetNativeDisplayHandle();

    XUnmapWindow(display, pimpl_->window);
}

void Window::Show()
{
    assert(DoesExist());

    Display* display = GetNativeDisplayHandle();

    XMapWindow(display, pimpl_->window);
}

void Window::Destroy()
{
    assert(DoesExist());

    Display* d = GetNativeDisplayHandle();

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

void Window::Invalidate()
{
    assert(DoesExist());

    Display* d = GetNativeDisplayHandle();

    XClearArea(d, pimpl_->window,
        0, 0,
        GetSurfaceWidth(),
        GetSurfaceHeight(),
        True);
    XFlush(d);
}

void Window::Move(int x, int y)
{
    assert(DoesExist());
    assert(!IsFullscreen());

    Display* d = GetNativeDisplayHandle();

    XMoveWindow(d, pimpl_->window, x, y);
    XFlush(d);
}

void Window::SetFullscreen(bool fullscreen)
{
    assert(DoesExist());

    if (fullscreen == pimpl_->fullscreen)
        return;

    Display* d = GetNativeDisplayHandle();
    ::Window   w = GetNativeHandle();

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
        XGrabPointer(d, w, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, w, X11_None, CurrentTime);
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

void Window::ShowCursor(bool on)
{
    assert(DoesExist());
    Display* d = GetNativeDisplayHandle();
    if (on)
    {
        XUndefineCursor(GetNativeDisplayHandle(), pimpl_->window);
    }
    else
    {
        static char null[] = { 0,0,0,0};
        Cursor invisibleCursor;
        Pixmap bitmapNoData;
        XColor black  = {0};
        Pixmap pixmap = XCreateBitmapFromData(d, pimpl_->window, null, 1, 1);
        Cursor cursor = XCreatePixmapCursor(d, pixmap, pixmap, &black, &black, 0, 0);
        XDefineCursor(d, pimpl_->window, cursor);
        XFreeCursor(d, cursor);
        XFreePixmap(d, pixmap);
    }
    pimpl_->cursor = on;
    XFlush(d);
}

bool Window::GrabMouse(bool on_off)
{
    auto* display = GetNativeDisplayHandle();
    if (on_off)
    {
        const auto event_mask = ButtonPressMask | ButtonReleaseMask |
                                EnterWindowMask | LeaveWindowMask |
                                PointerMotionMask | ButtonMotionMask;
        // okay.. so there's no way to figure out what is the current cursor ?
        // and this stupid API requires a cursor to be given.. (why in the f*?)
        const auto cursor = pimpl_->cursor ? XCreateFontCursor(display, 2) : X11_None;
        return XGrabPointer(display,
                            pimpl_->window, False, event_mask,
                            GrabModeAsync, GrabModeAsync,
                            pimpl_->window, cursor,
                            CurrentTime) == Success;
    }
    else
    {
        return XUngrabPointer(display, CurrentTime) == Success;
    }
    pimpl_->mouse_grab = on_off;
}

void Window::SetFocus()
{
    assert(DoesExist());

    Display* d = GetNativeDisplayHandle();

    XRaiseWindow(d, pimpl_->window);
    XSetInputFocus(d, pimpl_->window, X11_RevertToNone, CurrentTime);
    XFlush(d);
}

void Window::SetSize(uint_t width, uint_t height)
{
    assert(width);
    assert(height);
    assert(DoesExist());
    assert(!IsFullscreen());

    Display* d = GetNativeDisplayHandle();

    XResizeWindow(d, pimpl_->window, width, height);
    XFlush(d);
}

void Window::SetEncoding(Encoding enc)
{
    pimpl_->enc = enc;
}

void Window::SetTitle(const std::string& title)
{
    assert(DoesExist());

    Display* d = GetNativeDisplayHandle();

    // todo: work out the actual encoding. we could have to do
    // a conversion from UTF-8 to "Host portable character encoding"
    // in case host is using something other than UTF-8
    XStoreName(d, pimpl_->window, title.c_str());
    XFlush(d);
}

bool Window::ProcessEvent(const native_event_t& ev)
{
    if (ev.get_window_handle() != GetNativeHandle())
        return false;

    const XEvent& event = ev;

    switch (event.type)
    {
        case MotionNotify:
            if (OnMouseMove)
            {
                WindowEventMouseMove mickey = {};
                mickey.window_x = event.xmotion.x;
                mickey.window_y = event.xmotion.y;
                mickey.global_x = event.xmotion.x_root;
                mickey.global_y = event.xmotion.y_root;
                OnMouseMove(mickey);
            }
            break;

        case ButtonPress:
            if (OnMousePress)
            {
                const auto& button = TranslateMouseButtonEvent(ev);

                WindowEventMousePress mickey = {};
                mickey.window_x = event.xbutton.x;
                mickey.window_y = event.xbutton.y;
                mickey.global_x = event.xbutton.x_root;
                mickey.global_y = event.xbutton.y_root;
                mickey.modifiers = button.first;
                mickey.btn       = button.second;
                OnMousePress(mickey);
            }
            break;

        case ButtonRelease:
            if (OnMouseRelease)
            {
                const auto& button = TranslateMouseButtonEvent(ev);

                WindowEventMouseRelease mickey = {};
                mickey.window_x = event.xbutton.x;
                mickey.window_y = event.xbutton.y;
                mickey.global_x = event.xbutton.x_root;
                mickey.global_y = event.xbutton.y_root;
                mickey.modifiers = button.first;
                mickey.btn       = button.second;
                OnMouseRelease(mickey);
            }
            break;

        case FocusIn:
            if (OnGainFocus)
                OnGainFocus(WindowEventGainFocus{});
            break;

        case FocusOut:
            if (OnLostFocus)
                OnLostFocus(WindowEventLostFocus{});
            break;

        case Expose:
            if (OnPaint)
            {
                WindowEventPaint paint = {0};
                paint.x      = event.xexpose.x;
                paint.y      = event.xexpose.y;
                paint.width  = event.xexpose.width;
                paint.height = event.xexpose.height;
                OnPaint(paint);
            }
            break;

        case ConfigureNotify:
            if (pimpl_->width != event.xconfigure.width || pimpl_->height != event.xconfigure.height)
            {
                pimpl_->width  = event.xconfigure.width;
                pimpl_->height = event.xconfigure.height;
                if (OnResize)
                {
                    WindowEventResize resize = {0};
                    resize.width  = event.xconfigure.width;
                    resize.height = event.xconfigure.height;
                    OnResize(resize);
                }
            }
            break;

        case CreateNotify:
            if (OnCreate)
            {
                WindowEventCreate create = {0};
                create.x      = event.xcreatewindow.x;
                create.y      = event.xcreatewindow.y;
                create.width  = event.xcreatewindow.width;
                create.height = event.xcreatewindow.height;
                OnCreate(create);
            }
            break;


        case ClientMessage:
            if ((Atom)event.xclient.data.l[0] == WM_DELETE_WINDOW)
            {
                if (OnWantClose)
                    OnWantClose(WindowEventWantClose{});
            }
            break;

        case KeyPress:
            if (OnKeyDown)
            {
                const auto& keys = TranslateKeydownEvent(ev);
                if (keys.second != Keysym::None)
                    OnKeyDown(WindowEventKeyDown{keys.second, keys.first});
            }
            if (OnChar)
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
                hack.xmap.window = GetNativeHandle();
                XSendEvent(event.xany.display, GetNativeHandle(), False, 0, &hack);
            }
            break;

        case KeyRelease:
            if (OnKeyUp)
            {
                const auto& keys = TranslateKeydownEvent(ev);
                if (keys.second != Keysym::None)
                    OnKeyUp(WindowEventKeyUp{keys.second, keys.first});
            }
            break;

        case MapNotify:
            if (!event.xany.send_event)
                break;

            if (OnChar)
            {
                const XMapEvent& uchar = event.xmap;
                const long ucs2 = (long)uchar.event;

                WindowEventChar c = {0};

                if (pimpl_->enc == Encoding::ASCII)
                    c.ascii = ucs2 & 0x7f;
                else if (pimpl_->enc == Encoding::UCS2)
                    c.ucs2 = ucs2;
                else if (pimpl_->enc == Encoding::UTF8)
                    enc::utf8_encode(&ucs2, &ucs2 + 1, &c.utf8[0]);

                OnChar(c);
            }
            break;

    }
    return true;
}

uint_t Window::GetSurfaceWidth() const
{
    assert(DoesExist());

    Display* d = GetNativeDisplayHandle();

    XWindowAttributes attrs;
    XGetWindowAttributes(d, pimpl_->window, &attrs);

    return attrs.width;
}

uint_t Window::GetSurfaceHeight() const
{
    assert(DoesExist());

    Display* d = GetNativeDisplayHandle();

    XWindowAttributes attrs;
    XGetWindowAttributes(d, pimpl_->window, &attrs);

    return attrs.height;
}

int Window::GetPosX() const
{
    assert(DoesExist());

    ::Display* d = GetNativeDisplayHandle();

    ::Window root_window = X11_None;
    ::Window parent_window = X11_None;
    ::Window* child_windows = nullptr;
    unsigned int num_children = 0;
    ::XQueryTree(d, pimpl_->window, &root_window, &parent_window, &child_windows, &num_children);
    if (child_windows)
        ::XFree(child_windows);

    assert(root_window == RootWindow(d, DefaultScreen(d)));

    ::Window the_window = pimpl_->window;

    // the big problem here is that our window can have been reparented
    // by the WM and the parent window *might* (or might not) contain
    // decorations such as border / title bar depending on the window manager.
    // Mapping the 0, 0 coordinate using our Window handle might not then map
    // to the top left corner of the "window" that user sees but refers only
    // to the drawable client area inside the window captions.
    //
    // Some potential ways to try to "fix" could be to try to query the WM
    // for window frame extents (but how to get the title bar?)
    // or then try to look for a parent window of this window and try to
    // use that as the window to map the coordinate from into desktop coordinate.
    //
    // Both are likely to fail randomly under different window manager systems.
    //

    // maybe the WM has reparanted the window and the WM parent window
    // includes "our" window and some decorations.
    if (parent_window != root_window)
        the_window = parent_window;

    ::Window child_dummy = X11_None;
    int x = 0;
    int y = 0;

    // Translate the position 0,0 in our target window to root window coordinate space.
    ::XTranslateCoordinates(d, the_window, root_window, 0, 0, &x, &y, &child_dummy);

    Atom actual_type;
    int actual_format = 0;
    unsigned long nitems, bytes_after;
    unsigned char *data = nullptr;
    if (::XGetWindowProperty(d, pimpl_->window,
        ::XInternAtom(d, "_NET_FRAME_EXTENTS", 0),
        0, 4, False, AnyPropertyType,
        &actual_type, &actual_format,
        &nitems, &bytes_after, &data) == Success)
    {
        const auto border_left   = ((const uint32_t*)data)[0];
        const auto border_right  = ((const uint32_t*)data)[1];
        const auto border_top    = ((const uint32_t*)data)[2];
        const auto border_bottom = ((const uint32_t*)data)[3];
        ::XFree(data);

        x = x - border_left;
    }
    return x;
}

int Window::GetPosY() const
{
    assert(DoesExist());

    ::Display* d = GetNativeDisplayHandle();

    ::Window root_window = X11_None;
    ::Window parent_window = X11_None;
    ::Window* child_windows = nullptr;
    unsigned int num_children = 0;
    ::XQueryTree(d, pimpl_->window, &root_window, &parent_window, &child_windows, &num_children);
    if (child_windows)
        ::XFree(child_windows);

    assert(root_window == RootWindow(d, DefaultScreen(d)));

    ::Window the_window = pimpl_->window;

    // the big problem here is that our window can have been reparented
    // by the WM and the parent window *might* (or might not) contain
    // decorations such as border / title bar depending on the window manager.
    // Mapping the 0, 0 coordinate using our Window handle might not then map
    // to the top left corner of the "window" that user sees but refers only
    // to the drawable client area inside the window captions.
    //
    // Some potential ways to try to "fix" could be to try to query the WM
    // for window frame extents (but how to get the title bar?)
    // or then try to look for a parent window of this window and try to
    // use that as the window to map the coordinate from into desktop coordinate.
    //
    // Both are likely to fail randomly under different window manager systems.
    //

    // maybe the WM has reparanted the window and the WM parent window
    // includes "our" window and some decorations.
    if (parent_window != root_window)
        the_window = parent_window;

    ::Window child_dummy = X11_None;
    int x = 0;
    int y = 0;

    // Translate the position 0,0 in our target window to root window coordinate space.
    ::XTranslateCoordinates(d, the_window, root_window, 0, 0, &x, &y, &child_dummy);

    Atom actual_type;
    int actual_format = 0;
    unsigned long nitems, bytes_after;
    unsigned char *data = nullptr;
    if (::XGetWindowProperty(d, pimpl_->window,
        ::XInternAtom(d, "_NET_FRAME_EXTENTS", 0),
        0, 4, False, AnyPropertyType,
        &actual_type, &actual_format,
        &nitems, &bytes_after, &data) == Success)
    {
        const auto border_left   = ((const uint32_t*)data)[0];
        const auto border_right  = ((const uint32_t*)data)[1];
        const auto border_top    = ((const uint32_t*)data)[2];
        const auto border_bottom = ((const uint32_t*)data)[3];
        ::XFree(data);

        y = y - border_top;
    }
    return y;
}

bool Window::DoesExist() const
{
    return pimpl_->window != 0;
}

bool Window::IsFullscreen() const
{
    return pimpl_->fullscreen;
}


Window::Encoding Window::GetEncoding() const
{
    return pimpl_->enc;
}

native_window_t Window::GetNativeHandle() const
{
    return native_window_t {pimpl_->window};
}

std::pair<uint_t, uint_t> Window::GetMinSize() const
{
    assert(DoesExist());

    Display* d = GetNativeDisplayHandle();

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

std::pair<uint_t, uint_t> Window::GetMaxSize() const
{
    assert(DoesExist());

    Display* d = GetNativeDisplayHandle();

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

