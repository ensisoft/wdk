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
#include <windowsx.h> // for GET_{X,Y}_LPARAM (mouse pointer)

#include <stdexcept>
#include <cassert>
#include <limits>

#include "wdk/events.h"
#include "wdk/window.h"
#include "wdk/system.h"
#include "wdk/utf8.h"
#include "msgqueue.h"

#pragma comment(lib, "User32.lib")

namespace wdk
{

// see the comments in system.cpp (PeekEvent) about this fiber stuff here.
extern LPVOID caller_fiber_handle;

struct Window::impl {
    HWND window = NULL;
    Encoding enc = Encoding::UTF8;
    bool fullscreen = false;
    bool resizing = false;
    bool show_cursor = true;
    bool was_cursor_shown = true;
    bool has_mouse_focus = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    DWORD style = 0;
    DWORD exstyle = 0;
    // a bit of a hack, we store here temporarily the latest
    // dirty paint rectangle, so tat we don't need to pass
    // any kind of heap allocated object from the wndproc
    // to the window's process message function.
    // this would cause a leak if the message got lost
    RECT rcPaint;

    static
    LRESULT CALLBACK WindowMessageProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        // windows has posted and sent messages. Functions pumping the event
        // queue (GetMessage, PeekMessage) dispatch (call repeatedly) the window's
        // WndProc, untill a *posted* message is retrieved from the queue.
        // in order to support the dispatching mechanism, where the client
        // retrieves a message from the display, then potentially filters it, ignores it or
        // dispatches it to window we put the message into own (global) message queue
        // that is then inspected by the calls to WaitEvent, PeekEvent.

        switch (msg)
        {
            // see the comments in PeekEvent implementation about this timer fiber shit here!
            case WM_ENTERSIZEMOVE:
                SetTimer(hwnd, 1, USER_TIMER_MINIMUM, NULL);
                return 0;
            case WM_EXITSIZEMOVE:
                KillTimer(hwnd, 1);
                return 0;
            case WM_TIMER:
                SwitchToFiber(caller_fiber_handle);
                return 0;

            case WM_CREATE:
               {
                   CREATESTRUCT* original = reinterpret_cast<CREATESTRUCT*>(lp);
                   CREATESTRUCT* copy = new CREATESTRUCT(*original);
                   wdk::impl::PutGlobalWindowMessage(hwnd, WM_CREATE, wp, (LPARAM)copy);
               }
               return 0;

            case WM_KILLFOCUS:
            case WM_SETFOCUS:
            case WM_SIZE:
            case WM_CLOSE:
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_MOUSELEAVE:
            case WM_MOUSEMOVE:
            case WM_MOUSEWHEEL:
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_CHAR:
                wdk::impl::PutGlobalWindowMessage(hwnd, msg, wp, lp);
                return 0;

            // sent to window when it's size, pos, etc are about to change.
            // need to handle this in case another application tries to resize
            // the window with MoveWindow etc.
            case WM_WINDOWPOSCHANGING: // returns zero
                {
                    const LONG style_bits = GetWindowLong(hwnd, GWL_STYLE);
                    if (style_bits & WS_SIZEBOX)
                        break;

                    WINDOWPOS* pos = (WINDOWPOS*)lp;
                    pos->flags |= SWP_NOSIZE;
                }
                return 0;

            // Make sure we always validate the window rect by calling
            // BeginPaint/EndPaint. The system will keep sending WM_PAINT
            // until the current "update region is validated".
            // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-validaterect
            case WM_PAINT:
                {
                    auto lptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
                    auto* self = reinterpret_cast<Window::impl*>(lptr);
                    PAINTSTRUCT paint = {};
                    RECT new_paint_rect = { 0 };
                    BeginPaint(hwnd, &paint);
                    UnionRect(&new_paint_rect, &self->rcPaint, &paint.rcPaint);
                    EndPaint(hwnd, &paint);
                    self->rcPaint = new_paint_rect;
                }
                wdk::impl::PutGlobalWindowMessage(hwnd, WM_PAINT, wp, lp);
                return 0;

                // ignored. expectation is that everything is painted at one go.
            case WM_ERASEBKGND:
                return 1;

            default:
            break;
        }
        return DefWindowProc(hwnd, msg, wp, lp);
    }

};

Window::Window() : pimpl_(new impl)
{
    pimpl_->window     = NULL;
    pimpl_->enc        = Encoding::UTF8;
    pimpl_->fullscreen = false;
    pimpl_->resizing   = false;
    pimpl_->x          = 0;
    pimpl_->y          = 0;
    pimpl_->style      = 0;
    pimpl_->exstyle    = 0;
    pimpl_->rcPaint    = RECT{ 0 };

    WNDCLASSEX cls    = {0};
    cls.cbSize        = sizeof(cls);
    cls.hInstance     = GetModuleHandle(NULL);
    cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    cls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    cls.style         = CS_DBLCLKS | CS_OWNDC;
    cls.lpfnWndProc   = impl::WindowMessageProc;
    cls.lpszClassName = TEXT("WDK-WINDOW");
    if (!RegisterClassEx(&cls))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            throw std::runtime_error("registerclassex failed");
    }

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
    assert(!DoesExist());

    // notes about the visualid.
    // on Windows the visualid (i.e. pixelformat id) is specific to a HDC.
    // and without the HDC is kinda useless.
    // Also Windows let's us only set the PixelFormat once per HDC.
    // Thus we let that be "unset" for now, and only if the window
    // is placed into a OpenGL rendering context as a rendering surface
    // we actually lock down on the PixelFormat.
    //
    // The other way around, i.e. creating the opengl context based
    // on the Window, we'd need to use the visualid and the HDC
    // to load the PIXELFORMATDESCRIPTOR and use that as the selection
    // criteria for choosing the config.
    //
    // What this really means is that the visualid should really be a
    // a type other than just an uint.
    // In fact on windows it should be a PIXELFORMATDESCRIPTOR*
    //
    // However this "abstraction" breaks down when used with EGL
    // since it only has a concept of an EGLint for a visualid.
    //
    // So what this really means is that on Windows there's no way to
    // create an OpenGL rendering context based on a Window portably.

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

    auto win = MakeUniqueHandle(CreateWindowEx(
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

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pimpl_.get());
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)impl::WindowMessageProc);

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

void Window::Hide()
{
    assert(DoesExist());

    ShowWindow(pimpl_->window, SW_HIDE);
}

void Window::Show()
{
    assert(DoesExist());

    ShowWindow(pimpl_->window, SW_SHOW);
}

void Window::Destroy()
{
    assert(DoesExist());

    const BOOL ret = DestroyWindow(pimpl_->window);

    assert(ret);

    pimpl_->window = NULL;
}

void Window::Invalidate()
{
    assert(DoesExist());

    InvalidateRect(pimpl_->window, NULL, TRUE);
}

void Window::Move(int x, int y)
{
    assert(DoesExist());

    RECT rc;
    GetWindowRect(pimpl_->window, &rc);
    SetWindowLongPtr(pimpl_->window, GWLP_WNDPROC, (LONG_PTR)DefWindowProc);
    MoveWindow(pimpl_->window, x, y, rc.right - rc.left, rc.bottom - rc.top, FALSE);
    SetWindowLongPtr(pimpl_->window, GWLP_WNDPROC, (LONG_PTR)impl::WindowMessageProc);
}

void Window::SetFullscreen(bool fullscreen)
{
    assert(DoesExist());

    if (fullscreen == pimpl_->fullscreen)
        return;

    HWND hwnd = pimpl_->window;

    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)DefWindowProc);

    if (fullscreen)
    {

        RECT rc;
        GetWindowRect(hwnd, &rc);

        pimpl_->style = GetWindowLong(hwnd, GWL_STYLE);
        pimpl_->exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        pimpl_->x = rc.left;
        pimpl_->y = rc.top;
        pimpl_->w = GetSurfaceWidth(); //rc.right;
        pimpl_->h = GetSurfaceHeight(); //rc.bottom;

        ShowWindow(hwnd, SW_HIDE);
        SetWindowLong(hwnd, GWL_STYLE,   WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);

        DEVMODE mode = {0};
        mode.dmSize  = sizeof(mode);
        if (!EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &mode))
            throw std::runtime_error("failed to get current display device videomode");

        // Get rid of the start bar by enabling "fullscreen" mode
        mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
        if (ChangeDisplaySettings(&mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            throw std::runtime_error("videomode change failed");

        // Resize the window to occupy the whole screen dimensions
        ::MoveWindow(hwnd, 0, 0, mode.dmPelsWidth, mode.dmPelsHeight, TRUE);
        ::ShowWindow(hwnd, SW_SHOW);
        ::SetFocus(hwnd);
    }
    else
    {
        ::ShowWindow(hwnd, SW_HIDE);

        // restore start bar and restore to the "default mode".
        // Todo: what if've changed the display mode manually through system
        // change. Maybe we should use some appropriate settings here?
        ChangeDisplaySettings(NULL, 0);

        SetWindowLong(hwnd, GWL_STYLE, pimpl_->style);
        SetWindowLong(hwnd, GWL_EXSTYLE, pimpl_->exstyle);

        // MSDN tells us for MoveWindow that width and height
        // should be the width and the height of the client area.
        // However if we restore to the window size when going into
        // fullscreen the window *grows*, but if restoring to
        // *client* size then the window shrinks.
        MoveWindow(hwnd, pimpl_->x, pimpl_->y, pimpl_->w, pimpl_->h, TRUE);

        RECT wnd, client;
        GetClientRect(hwnd, &client);
        GetWindowRect(hwnd, &wnd);
        const int dx = (wnd.right - wnd.left) - client.right;
        const int dy = (wnd.bottom - wnd.top) - client.bottom;
        auto width  = pimpl_->w;
        auto height = pimpl_->h;
        if (width > client.right)
            width += dx;
        if (height > client.bottom)
            height += dy;
        if (width < dx)
            width += dx;
        if (height < dy)
            height += dy;
        ::MoveWindow(hwnd, pimpl_->x, pimpl_->y, width, height, TRUE);
        ::ShowWindow(hwnd, SW_SHOW);
        ::SetFocus(hwnd);
    }

    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)impl::WindowMessageProc);

    // Since we swapped in the DefWindowProc we didn't get a chance to handle the
    // resize message. Thus we're going to regenerate one so that the client is
    // properly notified of the window size change.
    RECT rc;
    GetClientRect(hwnd, &rc);
    PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));

    pimpl_->fullscreen = fullscreen;
}

void Window::ShowCursor(bool on)
{
    assert(DoesExist());
    if (pimpl_->show_cursor == on)
        return;
    pimpl_->show_cursor = on;
    if (!pimpl_->has_mouse_focus)
        return;

    CURSORINFO cursor = {0};
    cursor.cbSize = sizeof(CURSORINFO);
    pimpl_->was_cursor_shown = cursor.flags & CURSOR_SHOWING;
    ::ShowCursor(pimpl_->show_cursor);
}

void Window::SetFocus()
{
    assert(DoesExist());

    ::SetFocus(pimpl_->window);
}

void Window::SetSize(uint_t width, uint_t height)
{
    assert(DoesExist());

    HWND hwnd = pimpl_->window;

    RECT wnd, client;
    GetWindowRect(hwnd, &wnd);
    GetClientRect(hwnd, &client);

    // current x,y position remains the same
    const int x = wnd.left;
    const int y = wnd.right;

    // make sure our wndproc doesnt mess up with things, set to Default
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)DefWindowProc);

    // current window frame sizes.
    // MoveWindow takes the size params to mean the size of the window
    // we want them to mean the client area. Hence we're adding the current
    // frame size to the dimensions.
    const int frame_w = (wnd.right - wnd.left) - client.right;
    const int frame_h = (wnd.bottom - wnd.top) - client.bottom;
    MoveWindow(hwnd, x, y, width + frame_w, height + frame_h, TRUE);

    // restore our wndproc
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)impl::WindowMessageProc);

    // Since we swapped in the DefWindowProc we didn't get a chance to handle the
    // resize message. Thus we're going to regenerate one so that the client is
    // properly notified of the window size change.
    // note that we indeed do a GetClientRect here since it's possible that
    // the size operation has failed. For example the sizes violate the minium/maximum sizes
    // that the window system is able to support.
    // our current api has no error provisions for this.
    RECT rc;
    GetClientRect(hwnd, &rc);
    PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));
}

void Window::SetEncoding(Encoding enc)
{
    pimpl_->enc = enc;
}

void Window::SetTitle(const std::string& title)
{
    HWND hwnd = pimpl_->window;
    const auto& wide = enc::utf8_decode(title);
    SetWindowTextW(hwnd, wide.c_str());
}

bool Window::ProcessEvent(const native_event_t& ev)
{
    if (ev.get_window_handle() != GetNativeHandle())
        return false;

    const MSG& m = ev;

    switch (m.message)
    {
        case WM_SETFOCUS:
            if (on_gain_focus)
                on_gain_focus(WindowEventFocus{});
            break;

        case WM_KILLFOCUS:
            if (on_lost_focus)
                on_lost_focus(WindowEventFocus{});
            break;

        case WM_PAINT:
            if (on_paint)
            {
                RECT rcPaint = pimpl_->rcPaint;
                if (IsRectEmpty(&rcPaint))
					GetUpdateRect(m.hwnd, &rcPaint, FALSE);

                if (IsRectEmpty(&rcPaint))
                    GetClientRect(m.hwnd, &rcPaint);

                WindowEventPaint paint;
                paint.x = rcPaint.left;
                paint.y = rcPaint.top;
                paint.width = rcPaint.right - rcPaint.left;
                paint.height = rcPaint.bottom - rcPaint.top;
                on_paint(paint);
            }
            pimpl_->rcPaint = RECT{ 0 };
            break;

        case WM_SIZE:
            if (on_resize)
            {
                RECT rc;
                GetClientRect(m.hwnd, &rc);

                WindowEventResize resize;
                resize.width  = rc.right;
                resize.height = rc.bottom;
                on_resize(resize);
            }
            break;

        case WM_CREATE:
            {
                const CREATESTRUCT* ptr = reinterpret_cast<const CREATESTRUCT*>(m.lParam);
                WindowEventCreate create;
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
                on_want_close(WindowEventWantClose{});
            break;


        case WM_KEYDOWN:
            if (on_keydown)
            {
                const auto& keys = TranslateKeydownEvent(ev);
                if (keys.second != Keysym::None) {
                    WindowEventKeydown key;
                    key.modifiers = keys.first;
                    key.symbol = keys.second;
                    on_keydown(key);
                }
            }
            break;

        case WM_KEYUP:
            if (on_keyup)
            {
                const auto& keys = TranslateKeydownEvent(ev);
                if (keys.second != Keysym::None) {
                    WindowEventKeyup key;
                    key.modifiers = keys.first;
                    key.symbol = keys.second;
                    on_keyup(key);
                }
            }
            break;

        // try to restore the application's mouse cursor to whatever state
        // it was before this window got mouse focus.
        case WM_MOUSELEAVE:
            pimpl_->has_mouse_focus = false;
            ::ShowCursor(pimpl_->was_cursor_shown);
            break;

        // on first mouse move show/hide the application mouse cursor
        // in order to emulate per window based behavior.
        case WM_MOUSEMOVE:
            if (!pimpl_->has_mouse_focus) {
                pimpl_->has_mouse_focus = true;
                // get the current mouse cursor state.
                CURSORINFO cursor = {0};
                cursor.cbSize = sizeof(CURSORINFO);
                GetCursorInfo(&cursor);
                pimpl_->was_cursor_shown = cursor.flags & CURSOR_SHOWING;
                // set the mouse cursor state associated with *this* window
                ::ShowCursor(pimpl_->show_cursor);
                // start tracking mouse when mouse enters the window's client area
                // so that we can receive the WM_MOUSELEAVE event.
                // this needs to be done every time since every notification (WM_MOUSELEAVE)
                // turns the tracking off.
                TRACKMOUSEEVENT tracking = { 0 };
                tracking.cbSize = sizeof(TRACKMOUSEEVENT);
                tracking.dwFlags = TME_LEAVE; 
                tracking.hwndTrack = pimpl_->window;
                TrackMouseEvent(&tracking);
            }

            if (on_mouse_move)
            {
                const auto& button = TranslateMouseButtonEvent(ev);

                POINT global;
                GetCursorPos(&global);
                WindowEventMouseMove mickey = {};
                mickey.window_x  = GET_X_LPARAM(m.lParam);
                mickey.window_y  = GET_Y_LPARAM(m.lParam);
                mickey.global_x  = global.x;
                mickey.global_y  = global.y;
                mickey.modifiers = button.first;
                mickey.btn       = button.second;
                on_mouse_move(mickey);
            }
            break;

        case WM_MOUSEWHEEL:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            if (on_mouse_press)
            {
                const auto& button = TranslateMouseButtonEvent(ev);

                POINT global;
                GetCursorPos(&global);
                WindowEventMousePress mickey= {};
                mickey.window_x  = GET_X_LPARAM(m.lParam);
                mickey.window_y  = GET_Y_LPARAM(m.lParam);
                mickey.global_x  = global.x;
                mickey.global_y  = global.y;
                mickey.modifiers = button.first;
                mickey.btn       = button.second;
                on_mouse_press(mickey);
            }
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            if (on_mouse_release)
            {
                const auto& button = TranslateMouseButtonEvent(ev);

                POINT global;
                GetCursorPos(&global);
                WindowEventMouseRelease mickey = {};
                mickey.window_x  = GET_X_LPARAM(m.lParam);
                mickey.window_y  = GET_Y_LPARAM(m.lParam);
                mickey.global_x  = global.x;
                mickey.global_y  = global.y;
                mickey.modifiers = button.first;
                mickey.btn       = button.second;
                on_mouse_release(mickey);
            }
            break;

        case WM_CHAR:
            if (on_char)
            {
                const WPARAM utf16 = m.wParam;

                WindowEventChar c = {0};
                if (pimpl_->enc == Encoding::ASCII)
                    c.ascii = utf16 & 0x7f;
                else if (pimpl_->enc == Encoding::UCS2)
                    c.ucs2 = (std::uint16_t)(utf16 & 0xFFFF);
                else if (pimpl_->enc == Encoding::UTF8)
                    enc::utf8_encode(&utf16, &utf16 + 1, &c.utf8[0]);

                on_char(c);

            }
            break;

        default:
            return true;
    }
    return true;
}

uint_t Window::GetSurfaceWidth() const
{
    assert(DoesExist());

    RECT rc = {0};
    GetClientRect(pimpl_->window, &rc);
    return rc.right;
}

uint_t Window::GetSurfaceHeight() const
{
    assert(DoesExist());

    RECT rc = {0};
    GetClientRect(pimpl_->window, &rc);
    return rc.bottom;
}

int Window::GetPosX() const
{
    assert(DoesExist());

    RECT rc = {0};
    GetWindowRect(pimpl_->window, &rc);
    return rc.left;
}

int Window::GetPosY() const
{
    assert(DoesExist());

    RECT rc = {0};
    GetWindowRect(pimpl_->window, &rc);
    return rc.top;
}

bool Window::DoesExist() const
{
    return pimpl_->window != NULL;
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
    return pimpl_->window;
}

std::pair<uint_t, uint_t> Window::GetMinSize() const
{
    assert(DoesExist());

    HWND hwnd = pimpl_->window;

    const LONG style_bits = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style_bits & WS_SIZEBOX))
        return std::make_pair(GetSurfaceWidth(), GetSurfaceHeight());

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
    auto win = MakeUniqueHandle(CreateWindowEx(
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

std::pair<uint_t, uint_t> Window::GetMaxSize() const
{
    assert(DoesExist());

    HWND hwnd = pimpl_->window;

    const LONG style_bits = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style_bits & WS_SIZEBOX))
        return std::make_pair(GetSurfaceWidth(), GetSurfaceHeight());

    // a window can maximize to the size of the largest monitor
    // see info about WM_GETMINMAXINFO
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms632605%28v=vs.85%29.aspx
    // but none of that works as expected.
    //MINMAXINFO minmax = {0};
    ////SendMessage(hwnd, WM_GETMINMAXINFO, 0, (LPARAM)&minmax);
    //DefWindowProc(hwnd, WM_GETMINMAXINFO, 0, (LPARAM)&minmax);

    const LONG ex_style_bits = GetWindowLong(hwnd, GWL_EXSTYLE);

    auto win = MakeUniqueHandle(CreateWindowEx(
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
