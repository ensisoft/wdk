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
#include <X11/extensions/xf86vmode.h>
#include <sys/select.h>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cstring>      // for memcmp
#include <cassert>
#include "../display.h"
#include "../event.h"

namespace {
    void identify_event(const XEvent& xev, wdk::event& ev)
    {
        using namespace wdk;
        // layout for all message types for the first 4 entries is always the same. the 5th entry 
        // is the window handle of the window that receives the message, except for keymap notify events
        if (xev.type != KeymapNotify)
            ev.window = native_window_t {xev.xkey.window};

        switch (xev.type)
        {
            // identified window events
            case FocusIn:
                ev.type = event_type::window_gain_focus;
                break;
            case FocusOut:
                ev.type = event_type::window_lost_focus;
                break;
            case Expose:
                ev.type = event_type::window_paint;
                break;

            // resize, move, map/unmap, border size change
            case ConfigureNotify:
                ev.type = event_type::window_configure;
                break;

            // window was created, xcreatewindow.window is the
            // id of the newly created window
            case CreateNotify:
                ev.type = event_type::window_create;
                ev.window = native_window_t {xev.xcreatewindow.window};
            break;
            // window was destroyed
            case DestroyNotify:
                ev.type = event_type::window_destroy;
                ev.window = native_window_t {xev.xdestroywindow.window};
                break;

            // this could be multiple messages but we only use it for query close
            // at the moment
            case ClientMessage:
                ev.type = event_type::window_close;
                break;

            // identified keyboard events
            case KeyPress:
                ev.type = event_type::keyboard_keydown;
                break;
            case KeyRelease:
                ev.type = event_type::keyboard_keyup;
                break;

                // got an event but we dont know what it is
        } // switch
    }
} //


namespace wdk
{

struct display::impl {
    Display* disp;
    int      fileno;
    int      modecount;
    int      currentmode;
    XF86VidModeModeInfo** modes;
};

display::display()
{
    pimpl_.reset(new impl);
    pimpl_->disp = XOpenDisplay(nullptr);
    if (!pimpl_->disp)
        throw std::runtime_error("cannot open X display");
    pimpl_->fileno = XConnectionNumber(pimpl_->disp);
    pimpl_->currentmode = 0;
    
    // const int screen = DefaultScreen(pimpl_->disp);
    // const Window root = RootWindow(pimpl_->disp, screen);

    // XSetWindowAttributes attr = {};
    // attr.event_mask = StructureNotifyMask;
    // XChangeWindowAttributes(pimpl_->disp, root, CWEventMask, &attr);

    // query for supported videmodes
    // the first element is the current video mode
    XF86VidModeGetAllModeLines(pimpl_->disp, XDefaultScreen(pimpl_->disp), &pimpl_->modecount, &pimpl_->modes);
}

display::~display()
{
    // restore video mode to what it was originally if it has been changed.
    if (pimpl_->currentmode != 0)
    {
        const int screen = DefaultScreen(pimpl_->disp);

        XF86VidModeSwitchToMode(pimpl_->disp, screen, pimpl_->modes[0]);
        XF86VidModeSetViewPort(pimpl_->disp, screen, 0, 0);
    }
    XFree(pimpl_->modes);
    XCloseDisplay(pimpl_->disp);
}

videomode display::get_current_video_mode() const
{
    assert(pimpl_->currentmode >= 0 && pimpl_->currentmode < pimpl_->modecount);

    XF86VidModeModeInfo* mode = pimpl_->modes[pimpl_->currentmode];

    videomode vm = {0};
    vm.xres = mode->hdisplay;
    vm.yres = mode->vdisplay;
    vm.id   = native_vmode_t(pimpl_->currentmode);
    return vm;
}

void display::set_video_mode(native_vmode_t id)
{
    assert(id >= 0 && id < pimpl_->modecount);

    if (pimpl_->currentmode == id)
        return;
    
    const int screen = DefaultScreen(pimpl_->disp);

    XF86VidModeModeInfo* mode = pimpl_->modes[id];

    if (XF86VidModeSwitchToMode(pimpl_->disp, screen, mode) == False)
        throw std::runtime_error("videomode switch failed");

    if (XF86VidModeSetViewPort(pimpl_->disp, screen, 0, 0) == False)
        throw std::runtime_error("setviewport failed");

    pimpl_->currentmode = id;
}

std::vector<videomode> display::list_video_modes() const
{
    std::vector<videomode> modes;

    for (int i=0; i<pimpl_->modecount; ++i)
    {
        XF86VidModeModeInfo* mode = pimpl_->modes[i];
        videomode vm = {0};
        vm.xres = mode->hdisplay;
        vm.yres = mode->vdisplay;
        vm.id   = native_vmode_t(i);
        modes.push_back(vm);
    }
    return modes;
}


bool display::has_event() const
{
    // XPending flushes the output queue
    return (XPending(pimpl_->disp) != 0);
}

void display::get_event(event& ev)
{
    ev.window = wdk::NULL_WINDOW;
    ev.type   = event_type::none;

    // XNextEvent flushes the output queue
    XNextEvent(pimpl_->disp, &ev.ev);

    identify_event(ev.ev, ev);
}

bool display::peek_event(event& ev) const
{
    ev.window = wdk::NULL_WINDOW;
    ev.type   = event_type::none;

    if (!XPending(pimpl_->disp))
        return false;

    // XPeekEvent flushes the output queue
    XPeekEvent(pimpl_->disp, &ev.ev);

    identify_event(ev.ev, ev);
    
    return true;
}

bool display::wait_for_events(ms_t timeout) const
{
    native_handle_t fd = pimpl_->fileno;
    native_handle_t* signaled = nullptr;
    return wait_for_events(&fd, 1, &signaled, timeout);
}

bool display::wait_for_events(native_handle_t* handles, uint_t num_handles, native_handle_t** signaled, ms_t timeout) const
{
    fd_set read;
    fd_set write;
    FD_ZERO(&read);
    FD_ZERO(&write);

    native_handle_t max_fd = 0;

    for (uint_t i=0; i<num_handles; ++i)
    {
        FD_SET(handles[i], &read);
        FD_SET(handles[i], &write);
        max_fd = std::max(max_fd, handles[i]);
    }

    FD_SET(pimpl_->fileno, &read);
    FD_SET(pimpl_->fileno, &write);
    max_fd = std::max(max_fd, pimpl_->fileno);

    struct timeval tv = {0};
    tv.tv_usec = timeout * 1000;
    struct timeval* p = timeout == NO_TIMEOUT ? nullptr : &tv;

    if (::select(max_fd + 1, &read, &write, nullptr, p) == -1)
        throw std::runtime_error("wait failed");
    
    // on linux we get a list of all the objects that were signaled
    // (there could be more than one) but since windows limits this information
    // to either one handle (the first one signaled) or all handles (all signaled)
    // we provide the same semantics on linux as well
    for (uint_t i=0; i<num_handles; ++i)
    {
        if (FD_ISSET(handles[i], &read) || FD_ISSET(handles[i], &write))
        {
            *signaled = &handles[i];
            return true;
        }
    }
    return false;
}

uint_t display::width() const
{
    return DisplayWidth(pimpl_->disp, DefaultScreen(pimpl_->disp));
}

uint_t display::height() const
{
    return DisplayHeight(pimpl_->disp, DefaultScreen(pimpl_->disp));
}

native_display_t display::handle() const
{
    return pimpl_->disp;
}

void dispose(const event&)
{
    // nothing to be done here
}


} // wdk
