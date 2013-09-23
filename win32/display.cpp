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

#include <Windows.h>
#include <stdexcept>
#include <vector>
#include <deque>
#include <cassert>
#include <cstring>
#include <csetjmp>
#include "../display.h"
#include "../event.h"
#include "helpers.h"

namespace {
    struct dev_mode {
        DWORD dmBitsPerPel;
        DWORD dmPelsWidth;
        DWORD dmPelsHeight;
        DWORD dmDisplayFrequency;
    };

    bool identify_event(const MSG& m, wdk::event& ev)
    {
        using namespace wdk;

        ev.window = m.hwnd;

        switch (m.message)
        {

            // messages posted directly to the queue
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN: 
                ev.type = event_type::keyboard_keydown;
                break;

            case WM_SYSKEYUP:
            case WM_KEYUP: 
                ev.type = event_type::keyboard_keyup;
                break;

            case WM_CHAR: // by TranslateMessage
                ev.type = event_type::ime_char;
                break;

            case WM_PAINT:
                ev.type = event_type::window_paint;
                break;

            case WM_SETFOCUS:
                ev.type = event_type::window_gain_focus;
                break;

            case WM_KILLFOCUS:
                ev.type = event_type::window_lost_focus;
                break;

            case WM_CLOSE:
                ev.type = event_type::window_close;
                break;

            case WM_CREATE:            
                ev.type = event_type::window_create;
                break;

            case WM_SIZE:
                ev.type = event_type::window_configure;
                break;

            default:
                return false;
        }

        return true;

    } // identify_event

}  // namespace

namespace wdk
{


struct display::impl 
{
    HDC screen;
    std::vector<dev_mode> modes;
    size_t currentmode;
    size_t originalmode;
    HWND   window;
};

display::display()
{
    pimpl_.reset(new impl);
    pimpl_->screen = GetDC(NULL);
    pimpl_->currentmode = 0;
    pimpl_->originalmode = 0;

    DEVMODE current_dev_mode = {0};
    current_dev_mode.dmSize  = sizeof(current_dev_mode);

    // get current video mode
    if (!EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &current_dev_mode))
        throw std::runtime_error("failed to get current display device videomode");

    DWORD modeid = 0;
    DEVMODE dev  = {0};
    dev.dmSize   = sizeof(dev);

    bool found_current_mode = false;

    // list available video modes and try to find a mode matching
    // current video mode settings
    while (EnumDisplaySettings(nullptr, modeid, &dev))
    {
        if (current_dev_mode.dmBitsPerPel == dev.dmBitsPerPel &&
            current_dev_mode.dmPelsWidth  == dev.dmPelsWidth &&
            current_dev_mode.dmPelsHeight == dev.dmPelsHeight &&
            current_dev_mode.dmDisplayFrequency == dev.dmDisplayFrequency)
        {
            pimpl_->currentmode  = modeid;
            pimpl_->originalmode = modeid;
            found_current_mode   = true;
        }
        dev_mode mode = {0};
        mode.dmBitsPerPel = dev.dmBitsPerPel;
        mode.dmPelsWidth  = dev.dmPelsWidth;
        mode.dmPelsHeight = dev.dmPelsHeight;
        mode.dmDisplayFrequency = dev.dmDisplayFrequency;

        pimpl_->modes.push_back(mode);
        modeid++;
        std::memset(&dev, 0, sizeof(dev));
        dev.dmSize = sizeof(dev);
    }

    if (!found_current_mode)
        throw std::runtime_error("current videomode unknown");
}

display::~display()
{
    if (pimpl_->currentmode != pimpl_->originalmode)
    {
        const LONG ret = ChangeDisplaySettings(NULL, 0);

        assert(ret == DISP_CHANGE_SUCCESSFUL);
    }
    ReleaseDC(GetDesktopWindow(), pimpl_->screen);
}

videomode display::get_current_video_mode() const
{
    videomode mode = {0};
    mode.xres = pimpl_->modes[pimpl_->currentmode].dmPelsWidth;
    mode.yres = pimpl_->modes[pimpl_->currentmode].dmPelsHeight;
    mode.id   = native_vmode_t(pimpl_->currentmode);
    return mode;
}

void display::set_video_mode(native_vmode_t id)
{
    const DWORD index = (id == wdk::DEFAULT_VIDEO_MODE) ? pimpl_->originalmode : id;

    assert(index >= 0 && index < pimpl_->modes.size());

    DEVMODE mode = {0};
    mode.dmSize       = sizeof(mode);
    mode.dmPelsWidth  = pimpl_->modes[index].dmPelsWidth;
    mode.dmPelsHeight = pimpl_->modes[index].dmPelsHeight;
    mode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

    if (ChangeDisplaySettings(&mode, 0) != DISP_CHANGE_SUCCESSFUL)
        throw std::runtime_error("display mode change failed");

    pimpl_->currentmode = id;
}

std::vector<videomode> display::list_video_modes() const
{
    std::vector<videomode> modes;

    for (size_t i=0; i<pimpl_->modes.size(); ++i)
    {
        const dev_mode& dev = pimpl_->modes[i];

        videomode vm = {0};
        vm.xres = dev.dmPelsWidth;
        vm.yres = dev.dmPelsHeight;
        vm.id   = native_vmode_t(i);
        modes.push_back(vm);
    }

    return modes;
}

bool display::has_event() const
{
    MSG msg = {0};
    return (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE);
}

void display::get_event(event& ev)
{
    MSG& m = ev.ev;
    do
    {
        ev.type   = event_type::none;
        ev.window = wdk::NULL_WINDOW;

        BOOL ret = GetMessage(&m, NULL, 0, 0);
		assert(ret);

        if (!identify_event(m, ev))
            DefWindowProc(m.hwnd, m.message, m.wParam, m.lParam);
    }
    while (ev.type == event_type::none);
}

bool display::peek_event(event& ev) const
{
    MSG& m = ev.ev;
    do {
        ev.window = wdk::NULL_WINDOW;
        ev.type   = event_type::none;

        if (!PeekMessage(&m, nullptr, 0, 0, PM_NOREMOVE))
            return false;
        if (!identify_event(m, ev))
		{
			GetMessage(&m, NULL, 0, 0);
            DefWindowProc(m.hwnd, m.message, m.wParam, m.lParam);
		}
    }
    while (ev.type == event_type::none);

    return true;
}

bool display::wait_for_events(ms_t timeout) const
{
    const DWORD milliseconds = (timeout == wdk::NO_TIMEOUT) ? INFINITE : timeout;

    // neither PeekMessage or WaitMessage support timeouts
    // so we'll use MsgWaitForMultipleObjects 
    HANDLE dummy = NULL;
    DWORD ret = MsgWaitForMultipleObjects(0, &dummy, FALSE, milliseconds, QS_ALLEVENTS);
    if (ret == WAIT_FAILED)
        throw std::runtime_error("wait for events failed");
    else if (ret == WAIT_TIMEOUT)
        return false;
    return true;
}

bool display::wait_for_events(native_handle_t* handles, uint_t num_handles, native_handle_t** signaled, ms_t timeout) const
{
    const DWORD milliseconds = (timeout == wdk::NO_TIMEOUT) ? INFINITE : timeout;

    const DWORD ret = MsgWaitForMultipleObjects(num_handles, handles, FALSE, milliseconds, QS_ALLEVENTS);
    if (ret == WAIT_FAILED)
        throw std::runtime_error("wait for events failed");
    else if (ret == WAIT_TIMEOUT)
        return false;
    else if (ret >= WAIT_OBJECT_0 && (ret <= WAIT_OBJECT_0 + num_handles -1))
    {
        *signaled = &handles[ret - WAIT_OBJECT_0];
        return true;
    }

    // todo: handle this case
    assert(!"WAIT_ABANDONED_0");
    return true;
}

uint_t display::width() const
{
    return get_desktop_width();
}

uint_t display::height() const
{
    return get_desktop_height();
}

native_display_t display::handle() const
{
    return pimpl_->screen;
}

void dispose(const event& ev)
{
    const MSG& m = ev.ev;
    if (m.message == WM_CREATE)
    {
        CREATESTRUCT* create = reinterpret_cast<CREATESTRUCT*>(m.lParam);
        delete create;
    }
}

} // wdk

