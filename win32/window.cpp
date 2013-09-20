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
#include <cstring>              // for memset
#include <functional>
#include "../window.h"
#include "../events.h"
#include "../event.h"
#include "../display.h"
#include "../utility.h"
#include "helpers.h"

namespace wdk
{

struct window::impl {
    HWND hwnd;
    HDC  display;
    bool fullscreen;
    bool resizing;
    int x, y;

    static
    LRESULT CALLBACK window_startup_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        switch (msg)
        {
            case WM_CREATE:
                {
                    CREATESTRUCT* original = reinterpret_cast<CREATESTRUCT*>(lp);
                    CREATESTRUCT* copy = new CREATESTRUCT(*original);
                    PostMessage(hwnd, WM_APP + 1, wp, (LPARAM)copy);
                }
                return 0;
        }
        return DefWindowProc(hwnd, msg, wp, lp);
    }

    static
    LRESULT CALLBACK window_message_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        // windows has posted and sent messages. Functions pumping the event
        // queue (GetMessage, PeekMessage) dispatch (call repeatedly) the window's
        // WndProc, untill a *posted* message is retrieved from the queue.

        // in order to support the dispatching mechanism, where the client
        // retrieves a message from the display, then potentially filters it, ignores it or
        // dispatches it to keyboard/window/mouse objects we *post* the message back
        // into the queue, so that it gets picked up by Get/PeekMessage on a later call.

        LONG_PTR lptr = GetWindowLong(hwnd, GWL_USERDATA);

        assert(lptr);

        window::impl* self = reinterpret_cast<window::impl*>(lptr);

        switch (msg) 
        {
            // these messages can be forwarded directly 
            case WM_CLOSE:
            case WM_KILLFOCUS:
            case WM_SETFOCUS:
                PostMessage(hwnd, msg, wp, lp);
                return 0;

            // sent to window when it's size, pos, etc are about to change.
            // need to handle this in case another application tries to resize
            // the window with MoveWindow etc.
            case WM_WINDOWPOSCHANGING:
                {
                    const LONG style_bits = GetWindowLong(hwnd, GWL_STYLE);
                    if (style_bits & WS_SIZEBOX)
                        break;

                    WINDOWPOS* pos = (WINDOWPOS*)lp;
                    pos->flags |= SWP_NOSIZE;
                }
                return 0;

            // resizing window generates (from DefWindowProc) multiple WM_SIZING, WM_SIZE and WM_PAINT
            // messages. posting these directly to the queue won't work as expected (will call back to wndproc directly).
            // so in case the window is being resized we just ignore these messages and notify once the resize is finished. (WM_EXITSIZEMOVE)
            // however these messages are also used when a window becomes unobscured or minimize/maximize buttons are hit.
            case WM_SIZE:
            case WM_PAINT:              
                if (self->resizing)
                    return 0;
                PostMessage(hwnd, msg, wp, lp);
                return 0;

            case WM_ENTERSIZEMOVE:
                {
                    RECT rc;
                    GetWindowRect(hwnd, &rc);
                    self->x = rc.right - rc.left;
                    self->y = rc.bottom - rc.top;
                    self->resizing = true;
                }
                return 0;

            case WM_EXITSIZEMOVE:
                {
                    self->resizing = false;
                    RECT rc;
                    GetWindowRect(hwnd, &rc);
                    if (self->x != (rc.right - rc.left) || self->y != (rc.bottom - rc.top))
                        PostMessage(hwnd, WM_SIZE, 0, 0); // post only a notification that size has changed.
                }
                break;
            
            default:
            break;

        }
        return DefWindowProc(hwnd, msg, wp, lp);
    }

};

window::window(const wdk::display& disp)
{
    pimpl_.reset(new impl);
    pimpl_->hwnd       = NULL;
    pimpl_->display    = disp.handle();
    pimpl_->fullscreen = false;
    pimpl_->resizing   = false;
    
    WNDCLASSEX cls    = {0};
    cls.cbSize        = sizeof(cls);
    cls.hInstance     = GetModuleHandle(NULL);
    cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    cls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    cls.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    cls.lpfnWndProc   = impl::window_startup_proc;
    cls.lpszClassName = TEXT("WDK-WINDOW");
    if (!RegisterClassEx(&cls))
        throw std::runtime_error("registerclassex failed");
}

window::~window()
{
    if (exists())
        close();
}

void window::create(const window::params& how)
{
    assert(!exists());

    DWORD new_style_bits  = WS_EX_APPWINDOW;
    DWORD old_style_bits  = WS_POPUP;
    DWORD surface_width   = how.width;
    DWORD surface_height  = how.height;

    if (how.fullscreen)
    {
        DEVMODE mode = {0};
        mode.dmSize  = sizeof(mode);
        if (!EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &mode))
            throw std::runtime_error("failed to get current display device videomode");

        if (ChangeDisplaySettings(&mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            throw std::runtime_error("videomode change failed");

        // todo: is this in device units or logical units?
        surface_width   = mode.dmPelsWidth;
        surface_height  = mode.dmPelsHeight;
    }
    else
    {
        if (how.props & window::HAS_BORDER)
        {
            old_style_bits = WS_SYSMENU | WS_BORDER;
        }
        if (how.props & window::CAN_RESIZE)
        {
            if (how.props & window::HAS_BORDER)
                old_style_bits |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

            old_style_bits |= WS_SIZEBOX;
        }
    }

    auto win = make_unique_ptr(CreateWindowEx(new_style_bits, TEXT("WDK-WINDOW"), how.title.c_str(), old_style_bits, 0, 0, surface_width, surface_height, NULL, NULL, NULL, NULL),DestroyWindow);
    if (!win.get())
        throw std::runtime_error("create window failed");

    HWND hwnd = win.get();

    if (how.visualid)
    {
        auto hdc = make_unique_ptr(GetDC(hwnd), std::bind(ReleaseDC, hwnd, std::placeholders::_1));

        PIXELFORMATDESCRIPTOR pxd = {0};
        if (!DescribePixelFormat(hdc.get(), how.visualid, sizeof(pxd), &pxd))
            throw std::runtime_error("incorrect visualid");

        if (!SetPixelFormat(hdc.get(), how.visualid, &pxd))
            throw std::runtime_error("set pixelformat failed");
    }

    SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)pimpl_.get());
    SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)impl::window_message_proc);

    if (how.fullscreen)
    {
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        ShowWindow(hwnd, SW_SHOW);
    }
    else
    {
        // resize window to match the drawable client area with the desired size
        // based on the difference by client and window size.
        RECT client, window;
        GetClientRect(hwnd, &client);
        GetWindowRect(hwnd, &window);

        // resize
        const int dx = (window.right - window.left) - client.right;
        const int dy = (window.bottom - window.top) - client.bottom;
        MoveWindow(hwnd, window.left, window.top, surface_width + dx, surface_height + dy, TRUE);

#ifndef _NDEBUG
        RECT rc;
        GetClientRect(hwnd, &rc);
        // assert(rc.bottom == surface_height);
        // assert(rc.right  == surface_width);
#endif

        // position in the middle of the screen 
        GetWindowRect(hwnd, &window);
        const int width  = window.right - window.left;
        const int height = window.bottom - window.top;

        const int desktop_width  = get_desktop_width();
        const int desktop_height = get_desktop_height();

        // reposition
        const int xpos = (desktop_width - width) / 2;
        const int ypos = (desktop_height - height) / 2;
        MoveWindow(hwnd, xpos, ypos, width, height, TRUE);

        // finally show window
        ShowWindow(hwnd, SW_SHOW);    
    }

    pimpl_->hwnd       = win.release();
    pimpl_->fullscreen = how.fullscreen;
    pimpl_->resizing   = false;
    pimpl_->x          = 0;
    pimpl_->y          = 0;
}


void window::close()
{
    assert(exists());

    if (pimpl_->fullscreen)
       ChangeDisplaySettings(NULL, 0);

    BOOL ret = DestroyWindow(pimpl_->hwnd);
    assert(ret);
    
    pimpl_->hwnd = NULL;
    pimpl_->fullscreen = false; 
}

uint_t window::width() const
{
    RECT rc = {0};
    GetWindowRect(pimpl_->hwnd, &rc);
    return rc.right - rc.left;
}

uint_t window::height() const
{
    RECT rc = {0};
    GetWindowRect(pimpl_->hwnd, &rc);
    return rc.bottom - rc.top;
}

uint_t window::surface_width() const
{
    RECT rc = {0};
    GetClientRect(pimpl_->hwnd, &rc);
    return rc.right;
}

uint_t window::surface_height() const
{
    RECT rc = {0};
    GetClientRect(pimpl_->hwnd, &rc);
    return rc.bottom;
}

uint_t window::visualid() const
{
    if (!pimpl_->hwnd)
        return 0;

    HDC hdc = GetDC(pimpl_->hwnd);

    const int pixelformat = GetPixelFormat(hdc);

    ReleaseDC(pimpl_->hwnd, hdc);

    return pixelformat;
}

bool window::exists() const
{
    return (pimpl_->hwnd != NULL);
}

bool window::dispatch_event(const event& ev)
{
    const MSG& m = ev.ev;

    assert(m.hwnd == ev.window);

    if (m.hwnd != pimpl_->hwnd)
        return false;

    switch (ev.type)
    {
        case event_type::window_gain_focus:
        {
            if (event_gain_focus)
            {
                window_event_focus focus = {0};
                focus.window  = m.hwnd;
                focus.display = display();
                event_gain_focus(focus);
            }
        }
        break;

        case event_type::window_lost_focus:
        {
            if (event_lost_focus)
            {
                window_event_focus focus = {0};
                focus.window  = m.hwnd;
                focus.display = display();
                event_lost_focus(focus);
            }
        }
        break;

        case event_type::window_paint:
        {
            PAINTSTRUCT ps = {};
            HDC hdc = BeginPaint(m.hwnd, &ps);
            if (event_paint)
            {
                window_event_paint paint = {0};
                paint.x       = ps.rcPaint.left;
                paint.y       = ps.rcPaint.top;
                paint.width   = ps.rcPaint.right - ps.rcPaint.left;
                paint.height  = ps.rcPaint.bottom - ps.rcPaint.top;
                paint.window  = m.hwnd;
                paint.display = display();

                event_paint(paint);
            }
            EndPaint(m.hwnd, &ps);
        }
        break;

        case event_type::window_configure:
        {
            if (event_resize)
            {
                // get new surface dimensions
                RECT rc;
                GetClientRect(m.hwnd, &rc);
                
                window_event_resize size = {0};
                size.width   = rc.right;
                size.height  = rc.bottom;
                size.window  = m.hwnd;
                size.display = display();

                event_resize(size);
            }
        }
        break;

        case event_type::window_create:
        {
            if (event_create)
            {
                const CREATESTRUCT* ptr = reinterpret_cast<CREATESTRUCT*>(m.lParam);
                window_event_create create = {0};
                create.x      = ptr->x;
                create.y      = ptr->y;
                create.width  = ptr->cx;
                create.height = ptr->cy;
                create.fullscreen = pimpl_->fullscreen;
                create.window  = m.hwnd;
                create.display = display();

                event_create(create);
            }
        }
        break;

        case event_type::window_destroy:
        {
            if (event_destroy)
            {
                window_event_destroy destroy = {0};
                destroy.window  = m.hwnd;
                destroy.display = display();

                event_destroy(destroy);
            }
        }
        break;

        case event_type::window_close:
        {
            if (event_query_close)
            {
                window_event_query_close e = { false, m.hwnd, display() };
                event_query_close(e);
                if (e.should_close)
                    close();
            }
        }
        break;

        default:
            return false;
    }
    return true;
}

native_window_t window::handle() const
{
    if (!pimpl_->hwnd)
        return (native_window_t)wdk::NULL_WINDOW;
    return pimpl_->hwnd;
}

native_display_t window::display() const
{
    if (!pimpl_->hwnd)
        return (native_display_t)0;

    return pimpl_->display;
}

} // wdk
