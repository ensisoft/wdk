#include <windows.h>
#include <cassert>
#include <cstring>              // for memset
#include <functional>
#include "window.h"
#include "events.h"
#include "event.h"

namespace wdk
{

struct window::impl {
    HWND hwnd;
    HDC  hdc;
    HDC  screen;
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

window::window(native_display_t disp)
{
    pimpl_.reset(new impl);
    pimpl_->hwnd       = NULL;
    pimpl_->hdc        = NULL;
    pimpl_->screen     = GetDC(GetDesktopWindow());
    pimpl_->fullscreen = false;
	pimpl_->resizing   = false;
	
    WNDCLASSEX cls    = {0};
    cls.cbSize        = sizeof(cls);
    cls.hInstance     = GetModuleHandle(NULL);
    cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    cls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    cls.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    cls.lpfnWndProc   = impl::window_startup_proc;
    cls.lpszClassName = "WDK-WINDOW";
    if (!RegisterClassEx(&cls))
        throw std::runtime_error("registerclassex failed");
}

window::~window()
{
    ReleaseDC(GetDesktopWindow(), pimpl_->screen);

	if (exists())
		close();
}

void window::create(const window_param& how)
{
    assert(!exists());

    DWORD new_style_bits = WS_EX_APPWINDOW;
    DWORD old_style_bits = WS_POPUP;
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
        if (how.props & wdk::WP_BORDER)
        {
            old_style_bits = WS_SYSMENU | WS_BORDER;
        }
        if (how.props & wdk::WP_RESIZE)
        {
            if (how.props & wdk::WP_BORDER)
                old_style_bits |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

            old_style_bits |= WS_SIZEBOX;
        }
    }

    HWND hwnd = CreateWindowEx(
        new_style_bits,
        "WDK-WINDOW",
        how.title.c_str(),
        old_style_bits,
        CW_USEDEFAULT,  // x pos 
        CW_USEDEFAULT,  // y pos
        surface_width,
        surface_height,
        NULL,   // parent HWND
        NULL,   // menu HMENU
        NULL,   // instance HINSTANCE
        NULL);   // window param LPVOID
    if (hwnd == NULL)
        throw std::runtime_error("create window failed");

    RECT client, window;
    GetClientRect(hwnd, &client);
    GetWindowRect(hwnd, &window);

    const int dx = (window.right - window.left) - client.right;
    const int dy = (window.bottom - window.top) - client.bottom;
    MoveWindow(hwnd, window.left, window.top, surface_width + dx, surface_height + dy, TRUE);

#ifdef _DEBUG
    RECT rc;
    GetClientRect(hwnd, &rc);
    assert(rc.bottom == surface_height);
    assert(rc.right  == surface_width);
#endif

    pimpl_->hdc  = GetDC(hwnd);
    pimpl_->hwnd = hwnd;
    pimpl_->fullscreen = how.fullscreen;
	pimpl_->x = window.right - window.left;
	pimpl_->y = window.bottom - window.top;
 
	SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)pimpl_.get());
	SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)impl::window_message_proc);

    ShowWindow(hwnd, SW_SHOW);    

	if (pimpl_->fullscreen)
	{
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
}


void window::close()
{
	assert(exists());

    if (pimpl_->fullscreen)
       ChangeDisplaySettings(NULL, 0);

    ReleaseDC(pimpl_->hwnd, pimpl_->hdc);

    BOOL ret = DestroyWindow(pimpl_->hwnd);
    assert(ret);
    
    pimpl_->hwnd = NULL;
    pimpl_->hdc  = NULL;
    pimpl_->fullscreen = false; 
}

uint_t window::width() const
{
	assert(exists());

	RECT rc;
    GetWindowRect(pimpl_->hwnd, &rc);
    return rc.right - rc.left;
}

uint_t window::height() const
{
	assert(exists());

	RECT rc;
    GetWindowRect(pimpl_->hwnd, &rc);
    return rc.bottom - rc.top;
}

uint_t window::surface_width() const
{
	assert(exists());

	RECT rc;
    GetClientRect(pimpl_->hwnd, &rc);
    return rc.right;
}

uint_t window::surface_height() const
{
	assert(exists());

    RECT rc;
    GetClientRect(pimpl_->hwnd, &rc);
    return rc.bottom;
}

uint_t window::visualid() const
{
	assert(exists());

    return 0;
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
                focus.handle = m.hwnd;
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
                focus.handle = m.hwnd;
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
                paint.handle  = m.hwnd;
                paint.surface = hdc;
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
                size.width = rc.right;
                size.height = rc.bottom;
                size.handle = m.hwnd;
                size.surface = surface();
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
                create.handle = m.hwnd;
                create.surface = surface();
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
                destroy.handle = m.hwnd;
                destroy.display = display();

                event_destroy(destroy);
            }
        }
        break;

        case event_type::window_close:
        {
            if (event_close)
            {
                window_event_close e = { true };
                event_close(e);
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

native_surface_t window::surface() const
{
    if (!pimpl_->hwnd)
        return (native_surface_t)wdk::NULL_SURFACE;
    return pimpl_->hdc;
}

native_display_t window::display() const
{
    if (!pimpl_->hwnd)
        return (native_display_t)0;
    return pimpl_->screen;
}

} // wdk
