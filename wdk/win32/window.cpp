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

#define NOMINMAX
#include <windows.h>
#include <stdexcept>
#include <cassert>
#include <limits>
#include "../window_events.h"
#include "../window.h"
#include "../system.h"
#include "../utf8.h"
#include "../config.h"

#pragma comment(lib, "User32.lib")

namespace wdk
{

struct window::impl {
    HWND window;
    encoding enc;
    bool fullscreen;
    bool resizing;
    int x, y;
    int w, h;
    DWORD style;
    DWORD exstyle;

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

window::window() : pimpl_(new impl)
{
    pimpl_->window     = NULL;
    pimpl_->enc        = encoding::utf8;
    pimpl_->fullscreen = false;
    pimpl_->resizing   = false;
    pimpl_->x          = 0;
    pimpl_->y          = 0;
    pimpl_->style      = 0;
    pimpl_->exstyle    = 0;

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
        destroy();
}

void window::create(const std::string& title, uint_t width, uint_t height, 
    bool can_resize, bool has_border, bool initially_visible, uint_t visualid)
{
    assert(width);
    assert(height);
    assert(!title.empty());
    assert(!exists());

    if (visualid == 0)
    {
        config::attributes attrs = config::DEFAULT;
        config conf(attrs);
        visualid = conf.visualid();
    }

    DWORD new_style  = WS_EX_APPWINDOW;
    DWORD old_style  = WS_POPUP;

    if (has_border)
    {
        old_style = WS_SYSMENU | WS_BORDER;
    }
    if (can_resize)
    {
        if (has_border)
            old_style |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

        old_style |= WS_SIZEBOX;
    }

    auto win = make_unique_ptr(CreateWindowEx(
        new_style, 
        TEXT("WDK-WINDOW"), 
        title.c_str(), 
        old_style, 
        CW_USEDEFAULT, CW_USEDEFAULT,
        width,
        height,
        NULL, NULL, NULL, NULL), DestroyWindow);
    if (!win.get())
        throw std::runtime_error("create window failed");

    HWND hwnd = win.get();

    if (visualid)
    {
        auto hdc = make_unique_ptr(GetDC(hwnd), std::bind(ReleaseDC, hwnd, std::placeholders::_1));

        PIXELFORMATDESCRIPTOR pxd = {0};
        if (!DescribePixelFormat(hdc.get(), visualid, sizeof(pxd), &pxd))
            throw std::runtime_error("incorrect visualid");

        if (!SetPixelFormat(hdc.get(), visualid, &pxd))
            throw std::runtime_error("set pixelformat failed");

    }

    SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)pimpl_.get());
    SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)impl::window_message_proc);

    // resize window to match the drawable client area with the desired size
    // based on the difference by client and window size. note that this might
    // not always work. for example if window is given width smaller than the 
    // minimum required width for a window with title bar it won't resize
    RECT client, window;
    GetClientRect(hwnd, &client);
    GetWindowRect(hwnd, &window);

    // resize
    const int dx = (window.right - window.left) - client.right;
    const int dy = (window.bottom - window.top) - client.bottom;
    MoveWindow(hwnd, window.left, window.top, width + dx, height + dy, TRUE);

    if (initially_visible)
    {
        // finally show window
        ShowWindow(hwnd, SW_SHOW);    
    }

    pimpl_->window     = win.release();
    pimpl_->fullscreen = false;
    pimpl_->resizing   = false;
    pimpl_->x          = 0;
    pimpl_->y          = 0;
}

void window::hide()
{
    ShowWindow(pimpl_->window, SW_HIDE);
}

void window::show()
{
    ShowWindow(pimpl_->window, SW_SHOW);
}

void window::destroy()
{
    assert(handle());

    const BOOL ret = DestroyWindow(pimpl_->window);

    assert(ret);

    pimpl_->window = NULL;
}

void window::move(int x, int y)
{
    assert(handle());

    RECT rc;
    GetWindowRect(pimpl_->window, &rc);

    SetWindowLongPtr(pimpl_->window, GWL_WNDPROC, (LONG_PTR)DefWindowProc);

    MoveWindow(pimpl_->window, x, y, rc.right - rc.left, rc.bottom - rc.top, FALSE);

    SetWindowLongPtr(pimpl_->window, GWL_WNDPROC, (LONG_PTR)impl::window_message_proc);
}

void window::set_fullscreen(bool fullscreen)
{
    assert(exists());

    if (fullscreen == pimpl_->fullscreen)
        return;

    HWND hwnd = pimpl_->window;

    SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)DefWindowProc);

    if (fullscreen)
    {

        RECT rc;
        GetWindowRect(hwnd, &rc);

        pimpl_->style = GetWindowLong(hwnd, GWL_STYLE);
        pimpl_->exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        pimpl_->x = rc.left;
        pimpl_->y = rc.top;
        pimpl_->w = surface_width();
        pimpl_->h = surface_height();

        ShowWindow(hwnd, SW_HIDE);
        SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);

        DEVMODE mode = {0};
        mode.dmSize  = sizeof(mode);
        if (!EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &mode))
            throw std::runtime_error("failed to get current display device videomode");

        mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
        if (ChangeDisplaySettings(&mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            throw std::runtime_error("videomode change failed");

        MoveWindow(hwnd, 0, 0, mode.dmPelsWidth, mode.dmPelsHeight, TRUE);

        ShowWindow(hwnd, SW_SHOW);
    }
    else
    {
        ShowWindow(hwnd, SW_HIDE);

        ChangeDisplaySettings(NULL, 0);

        SetWindowLong(hwnd, GWL_STYLE, pimpl_->style);
        SetWindowLong(hwnd, GWL_EXSTYLE, pimpl_->exstyle);

        MoveWindow(hwnd, pimpl_->x, pimpl_->y, pimpl_->w, pimpl_->h, TRUE);

        ShowWindow(hwnd, SW_SHOW);
    }

    SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)impl::window_message_proc);

    pimpl_->fullscreen = fullscreen;
}

void window::set_focus()
{
    assert(exists());

    SetFocus(pimpl_->window);
}

void window::set_size(uint_t width, uint_t height)
{
    assert(exists());

    HWND hwnd = pimpl_->window;

    RECT wnd, client;
    GetWindowRect(hwnd, &wnd);
    GetClientRect(hwnd, &client);
    // current x,y position remains the same
    const int x = wnd.left;
    const int y = wnd.right;

    // make sure our wndproc doesnt mess up with things, set to Default
    SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)DefWindowProc);

    const int dx = (wnd.right - wnd.left) - client.right;
    const int dy = (wnd.bottom - wnd.top) - client.bottom;

    // MoveWindow seems to have very strange semantics.
    // If window is shrunk then new width and height will be considered
    // the new client area size (surface size). Except that if a dimension
    // is less than the minimum size in that dimension including the border/title bar
    // then that extra has to included in that dimension.
    // Otherwise if window is grown the new size will be the *window* size including
    // borders and title bar. So if we want to grow to a certain *client* size
    // we must include the borders and title bar in the actual new size. 
    
    if (width > client.right)
        width += dx;
    if (height > client.bottom)
        height += dy;

    if (width < dx)
        width += dx;
    if (height < dy)
        height += dy;

    MoveWindow(hwnd, x, y, width, height, TRUE);

    // restore our wndproc
    SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)impl::window_message_proc);
}

void window::set_encoding(encoding enc)
{
    pimpl_->enc = enc;
}

void window::poll_one_event()
{
    if (!have_events())
        return;

    const auto& ev = get_event();
    if (ev.get_window_handle() != handle())
        return;

    process_event(ev);
}

void window::wait_one_event()
{
    const auto& ev = get_event();
    if (ev.get_window_handle() != handle())
        return;

    process_event(ev);
}

void window::process_all_events()
{
    while (have_events())
        wait_one_event();
}

void window::process_event(const native_event_t& ev)
{
    assert(ev.get_window_handle());    
    assert(ev.get_window_handle() == handle());

    const MSG& m = ev;

    // translate virtual key messages into unicode characters 
    // which are posted in WM_CHAR (UTF-16)
    // this works fine for BMP in the range 0x0000 - 0xD7FF, 0xE000 - 0xFFFF.
    // for values above 0xFFFF this is broken, but not an issue thus far.
    TranslateMessage(&m);

    switch (m.message)
    {
        case WM_SETFOCUS:
            if (on_gain_focus)
                on_gain_focus(window_event_focus{});
            break;

        case WM_KILLFOCUS:
            if (on_lost_focus)
                on_lost_focus(window_event_focus{});
            break;

        case WM_PAINT:
            if (on_paint)
            {
                RECT rcPaint;
                GetUpdateRect(m.hwnd, &rcPaint, FALSE);

                window_event_paint paint = {0};
                paint.x      = rcPaint.left;
                paint.y      = rcPaint.top;
                paint.width  = rcPaint.right - rcPaint.left;
                paint.height = rcPaint.bottom - rcPaint.top;
                on_paint(paint);

            }
            break;

        case WM_SIZE:
            if (on_resize)
            {
                RECT rc;
                GetClientRect(m.hwnd, &rc);

                window_event_resize resize = {0};
                resize.width  = rc.right;
                resize.height = rc.bottom;
                on_resize(resize);
            }        
            break;
        
        case WM_CREATE:
            {
                const CREATESTRUCT* ptr = reinterpret_cast<const CREATESTRUCT*>(m.lParam);
                window_event_create create = {0};
                create.x          = ptr->x;
                create.y          = ptr->y;
                create.width      = ptr->cx;
                create.height     = ptr->cy;
                delete ptr;
                if (on_create)
                    on_create(create);
            }
            break;

        case WM_CLOSE:
            if (on_want_close)
                on_want_close(window_event_want_close{});
            break;


        case WM_KEYDOWN:
            if (on_keydown)
            {
                const auto& keys = translate_keydown_event(ev);
                if (keys.second != keysym::none)
                    on_keydown(window_event_keydown{keys.second, keys.first});
            }
            break;

        case WM_KEYUP:
            // todo:
            break;


            // todo:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            break;

        case WM_CHAR:
            if (on_char)
            {
                const WPARAM utf16 = m.wParam;

                window_event_char c = {0};
                if (pimpl_->enc == encoding::ascii)
                    c.ascii = utf16 & 0x7f;
                else if (pimpl_->enc == encoding::ucs2)
                    c.ucs2 = utf16;
                else if (pimpl_->enc == encoding::utf8)
                    enc::utf8_encode(&utf16, &utf16 + 1, &c.utf8[0]);

                on_char(c);

            }
            break;

        default:
            return;
    }

    ev.set_done();
}

void window::sync_all_events()
{

}

uint_t window::surface_width() const
{
    assert(exists());

    RECT rc = {0};
    GetClientRect(pimpl_->window, &rc);
    return rc.right;
}

uint_t window::surface_height() const
{
    assert(exists());

    RECT rc = {0};
    GetClientRect(pimpl_->window, &rc);
    return rc.bottom;
}

bool window::exists() const
{
    return pimpl_->window != NULL;
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
    return pimpl_->window;
}

egl_handle_t window::egl_handle() const 
{
    return pimpl_->window;
}

uint_t window::visualid() const
{
    assert(exists());

    HDC hdc = GetDC(pimpl_->window);

    int pixelformat = GetPixelFormat(hdc);

    ReleaseDC(pimpl_->window,hdc);

    return pixelformat;
}

std::pair<uint_t, uint_t> window::min_size() const
{
    assert(exists());

    HWND hwnd = pimpl_->window;

    const LONG style_bits = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style_bits & WS_SIZEBOX))
        return std::make_pair(surface_width(), surface_height());

    // using GetSystemMetrics with SM_CXMINTRACK can't be correct
    // because that's only a single value. however how small the window
    // can be certainly depends on it's style. a borderless/captionless 
    // window can resize smaller than one with a title bar.

    // MINMAXINFO doesn't work either. it only has "ptMinTrackSize"
    // which equals values in SM_CXMINTRACK

    // this doesn't work doh
    // RECT min = {0};
    // min.right = 1;
    // min.bottom = 1;
    // AdjustWindowRectEx(&min, style_bits, FALSE, ex_style_bits);
       
    const LONG ex_style_bits = GetWindowLong(hwnd, GWL_EXSTYLE);

    // create a window with matching style and see what the actual size will be.
    auto win = make_unique_ptr(CreateWindowEx(
        ex_style_bits,
        TEXT("WDK-WINDOW"),
        TEXT(""),
        style_bits,
        0, 0,
        1, 1,
        NULL, NULL, NULL, NULL), DestroyWindow);
    assert(win.get());

    RECT rc = {0};
    GetClientRect(win.get(), &rc);

    if (rc.bottom == 0)
        rc.bottom = 1;
    if (rc.right == 0)
        rc.right = 1;

    return std::make_pair((uint_t)rc.right, (uint_t)rc.bottom);

}

std::pair<uint_t, uint_t> window::max_size() const
{
    assert(exists());

    HWND hwnd = pimpl_->window;

    const LONG style_bits = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style_bits & WS_SIZEBOX))
        return std::make_pair(surface_width(), surface_height());

    // a window can maximize to the size of the largest monitor
    // see info about WM_GETMINMAXINFO
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms632605%28v=vs.85%29.aspx
    // but none of that works as expected.
    //MINMAXINFO minmax = {0};
    ////SendMessage(hwnd, WM_GETMINMAXINFO, 0, (LPARAM)&minmax);
    //DefWindowProc(hwnd, WM_GETMINMAXINFO, 0, (LPARAM)&minmax);

    const LONG ex_style_bits = GetWindowLong(hwnd, GWL_EXSTYLE);

    auto win = make_unique_ptr(CreateWindowEx(
        ex_style_bits,
        TEXT("WDK-WINDOW"),
        TEXT(""),
        style_bits,
        0, 0,
        std::numeric_limits<int>::max(),
        std::numeric_limits<int>::max(),
        NULL, NULL, NULL, NULL), DestroyWindow);
    assert(win.get());

    RECT rc = {0};
    GetClientRect(win.get(), &rc);

    return std::make_pair((uint_t)rc.right, (uint_t)rc.bottom);
}

} // wdk
