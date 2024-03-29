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

#include <algorithm>
#include <cassert>

#include "wdk/system.h"
#include "wdk/videomode.h"
#include "wdk/keys.h"
#include "wdk/win32/msgqueue.h"

namespace {
    using namespace wdk;

    struct key_mapping {
        Keysym wdk;
        UINT   win;
    };
    bool operator<(const key_mapping& i, const key_mapping& j)
    {
        return i.wdk < j.wdk;
    }

    static key_mapping keymap[] = {
        {Keysym::None,          0},
        {Keysym::Backspace,     VK_BACK},
        {Keysym::Tab,           VK_TAB},
        {Keysym::Enter,         VK_RETURN},
        {Keysym::Space,         VK_SPACE},
        {Keysym::Key0,          0x30},
        {Keysym::Key1,          0x31},
        {Keysym::Key2,          0x32},
        {Keysym::Key3,          0x33},
        {Keysym::Key4,          0x34},
        {Keysym::Key5,          0x35},
        {Keysym::Key6,          0x36},
        {Keysym::Key7,          0x37},
        {Keysym::Key8,          0x38},
        {Keysym::Key9,          0x39},
        {Keysym::KeyA,          0x41},
        {Keysym::KeyB,          0x42},
        {Keysym::KeyC,          0x43},
        {Keysym::KeyD,          0x44},
        {Keysym::KeyE,          0x45},
        {Keysym::KeyF,          0x46},
        {Keysym::KeyG,          0x47},
        {Keysym::KeyH,          0x48},
        {Keysym::KeyI,          0x49},
        {Keysym::KeyJ,          0x4A},
        {Keysym::KeyK,          0x4B},
        {Keysym::KeyL,          0x4C},
        {Keysym::KeyM,          0x4D},
        {Keysym::KeyN,          0x4E},
        {Keysym::KeyO,          0x4F},
        {Keysym::KeyP,          0x50},
        {Keysym::KeyQ,          0x51},
        {Keysym::KeyR,          0x52},
        {Keysym::KeyS,          0x53},
        {Keysym::KeyT,          0x54},
        {Keysym::KeyU,          0x55},
        {Keysym::KeyV,          0x56},
        {Keysym::KeyW,          0x57},
        {Keysym::KeyX,          0x58},
        {Keysym::KeyY,          0x59},
        {Keysym::KeyZ,          0x5A},
        {Keysym::F1,            VK_F1},
        {Keysym::F2,            VK_F2},
        {Keysym::F3,            VK_F3},
        {Keysym::F4,            VK_F4},
        {Keysym::F5,            VK_F5},
        {Keysym::F6,            VK_F6},
        {Keysym::F7,            VK_F7},
        {Keysym::F8,            VK_F8},
        {Keysym::F9,            VK_F9},
        {Keysym::F10,           VK_F10},
        {Keysym::F11,           VK_F11},
        {Keysym::F12,           VK_F12},
        {Keysym::ControlR,      VK_RCONTROL},
        {Keysym::ControlL,      VK_LCONTROL},
        {Keysym::AltL,          VK_LMENU},
        //{keysym::alt_R,       VK_RMENU},
        {Keysym::ShiftL,        VK_LSHIFT},
        {Keysym::ShiftR,        VK_RSHIFT},
        {Keysym::CapsLock,      VK_CAPITAL},
        {Keysym::Insert,        VK_INSERT},
        {Keysym::Del,           VK_DELETE},
        {Keysym::Home,          VK_HOME},
        {Keysym::End,           VK_END},
        {Keysym::PageUp,        VK_PRIOR},
        {Keysym::PageDown,      VK_NEXT},
        {Keysym::ArrowLeft,     VK_LEFT},
        {Keysym::ArrowUp,       VK_UP},
        {Keysym::ArrowDown,     VK_DOWN},
        {Keysym::ArrowRight,    VK_RIGHT},
        {Keysym::Escape,        VK_ESCAPE},
        {Keysym::Plus,          VK_OEM_PLUS},
        {Keysym::Minus,         VK_OEM_MINUS}
    };
    struct table_sorter {
        table_sorter()
        {
            std::sort(std::begin(keymap), std::end(keymap));
        }
    } sort_my_data_bitch;

    UINT find_keysym(Keysym sym)
    {
        assert(sym != Keysym::None);

        const auto it = std::lower_bound(std::begin(keymap), std::end(keymap), key_mapping{sym, 0});

        // should be there
        assert(it != std::end(keymap));

        return (*it).win;
    }

    enum { KEY_DOWN = 0x8000 };

} // namespace

namespace wdk
{
native_display_t GetNativeDisplayHandle()
{
    class Display
    {
    public:
        Display()
        {
            WNDCLASSEX cls    = {0};
            cls.cbSize        = sizeof(cls);
            cls.lpfnWndProc   = Display::WndProc;
            cls.lpszClassName = TEXT("WDK-SYSTEM-WINDOW");
            RegisterClassEx(&cls);

            // In order to detect the display change notification we need
            // to have a valid window. We don't know if the user has created
            // any windows or not, so we'll create one here just to listen
            // for that message.
            m_wnd_system = CreateWindow(TEXT("WDK-SYSTEM-WINDOW"), TEXT(""),
                WS_POPUP, 0, 0, 1, 1,
                NULL, NULL, NULL, NULL);
            if (m_wnd_system == NULL)
                throw std::runtime_error("create system window failed");
            m_hdc_desktop = GetDC(NULL);

        }
       ~Display()
        {
            BOOL ret = TRUE;
            ret = ReleaseDC(GetDesktopWindow(), m_hdc_desktop);
            assert(ret == TRUE);
            ret = DestroyWindow(m_wnd_system);
            (void)ret;
        }
        HDC getDesktopHDC() const
        { return m_hdc_desktop; }
    private:
        static
        LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
        {
            if (msg == WM_DISPLAYCHANGE)
                wdk::impl::PutGlobalWindowMessage(hwnd, WM_DISPLAYCHANGE, wp, lp);
            return DefWindowProc(hwnd, msg, wp, lp);
        }

    private:
        HDC   m_hdc_desktop;
        HWND  m_wnd_system;
    };

    static Display disp;

    return disp.getDesktopHDC();
}

VideoMode GetCurrentVideoMode()
{
    DEVMODE cur_mode = {0};

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &cur_mode))
        throw std::runtime_error("failed to current display settings");

    VideoMode mode;
    mode.xres = cur_mode.dmPelsWidth;
    mode.yres = cur_mode.dmPelsHeight;
    return mode;

}

void SetVideoMode(const VideoMode& m)
{
    DEVMODE mode = {0};
    mode.dmSize       = sizeof(mode);
    mode.dmPelsWidth  = m.xres;
    mode.dmPelsHeight = m.yres;
    mode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

    // Call this function here so that we actually create
    // our window to listen for the notification about display change.
    // todo: should we be able to receive this notification *always*
    // without having to initiate display change ourselves?
    GetNativeDisplayHandle();

    if (ChangeDisplaySettings(&mode, 0) != DISP_CHANGE_SUCCESSFUL)
       throw std::runtime_error("display mode change failed");

}

std::vector<VideoMode> ListVideoModes()
{
    std::vector<VideoMode> modes;

    DEVMODE dev;
    DWORD modeid = 0;

    while (EnumDisplaySettings(NULL, modeid++, &dev))
    {
        VideoMode mode(dev.dmPelsWidth, dev.dmPelsHeight);

        if (std::find(modes.begin(), modes.end(), mode) == modes.end())
            modes.push_back(mode);
    }

    return modes;
}

LPVOID caller_fiber_handle = nullptr;
LPVOID msgloop_fiber_handle = nullptr;

void PeekEventFiberDelegate(LPVOID param)
{
    MSG m;

    while (true)
    {
        while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
        {
            // translate virtual key messages into unicode characters
            // which are posted in WM_CHAR (UTF-16)
            // this works fine for BMP in the range 0x0000 - 0xD7FF, 0xE000 - 0xFFFF.
            // for values above 0xFFFF this is broken, but not an issue thus far.
            TranslateMessage(&m);
            DispatchMessage(&m);
        }
        SwitchToFiber(caller_fiber_handle);
    }
}

bool PeekEvent(native_event_t& ev)
{
    // the fundamental problem we're trying to solve here is that when a window is 
    // being resized (or moved) the system generates a WM_SYSCOMMAND message 
    // (wparam = SC_MOVE/SC_SIZE). If/when this message is passed to the DefWindowProc
    // it enters a so called "modal loop" and does not return until the resize/move
    // operation has finished. While the window is being resized the modal loop 
    // invokes the WndProc directly with messages such as WM_ENTERSIZEMOVE, 
    // WM_EXITSIZEMOVE and WM_PAINT. 
    // The fact that the modal loop doesn't return however is bad news for an
    // interactive type of application (such as a game) that is running a continous
    // rendering loop and does not care about WM_PAINT message handler.
    // Since the main thread (or whichever thread is handling the windowing) is now
    // blocked the application's main loop cannot run and it cannot produce any frames
    // which means that its UI will appear "frozen" to the user. 
    //
    // There are some suggestions on the internet saying "use threads" but I think
    // this is a loaded footgun waiting to go off. Any sane Win32 app does not mix
    // threads with window objects.. this will certainly cause hard to fix bugs
    // since there is some (undocumented) amount of thread affinity per each window
    // object. Additionally the message loop we're trying to pump here is specific
    // to the thread.
    // Obivously the application could offload some work such as running simulations
    // or doing rendering to a separate GL context/thread but the main problem would
    // still be same, those threads' work could not be displayed while the main thread
    // is not able to render and put new pixels on the window surface.
    // 
    // One solution could be to possibly prevent this processing from happening
    // inside of DefWindowProc by catching WM_SYSCOMMAND. However this then means
    // that the window will not respond to resize or move requests. There might be
    // a way for the application itself to simulate that behaviour but it's probably
    // difficult to get right (I'm sure there are many pitfalls around this and 
    // undocumented behaviours).
    // 
    // So the current solution here is to convert the calling thread to a fiber 
    // on the first call. Then we delegate the message peeking to a secondary
    // fiber that continuously runs the message loop but only when yielded 
    // by the main fiber. Yielding to the message loop fiber stops the main fiber
    // until the message loop fiber yields which it does when messages are found.
    // Now the real trick here is that somehow when the message loop fiber 
    // runs and enters the modal loop it needs to intermittently yield to the 
    // main fiber as well! 
    // We can do this from the WndProc when the DefWindowProc calls it directly
    // during the processing! And the obvious choice could be WM_PAINT.
    // Except that this doesn't work so well. As is the case with Win32 there's
    // always that one more snag or undocumented thing among all the documented lies. 
    // It looks that the WM_PAINT is indeed invoked but for whatever reason
    // yielding to the main fiber from WM_PAINT when sandwitched between
    // ENTER/EXIT_SIZEMOVE does not work well... the result is very janky. 
    // Current best quess is that the frequency at which the WM_PAINT is generated
    // and WndProc called is not high enough. 
    // So instead of relying on that we use another hack which is to set a
    // WM_TIMER (also triggers WndProc being called) and yield from timer handler
    // to the main fiber. *phew*. 
    // 
    // This is still rather janky esp when resizing the window (moving seems to work
    // better) but at least it gives the application some chance to display something
    // even if the window is resized. The app's main loop should still not depend on 
    // this, rather all simulations and such step forward regardless of whether the 
    // application is able to put the pixels on the screen or not.
    //
    // Todo: investigate the overhead of fibers and whether there are ways to mitigate
    // by for example having minimal stack size.
    // Todo: skip the fiber creation if the window can never be moved/resized (i.e. in 
    // fullscreen mode always ?)

    if (caller_fiber_handle == nullptr) 
    {
        // this is now the apps "main" fiber.
        caller_fiber_handle = ConvertThreadToFiber(nullptr);
        // this will be message loop dispatching fiber.
        msgloop_fiber_handle = CreateFiber(0, PeekEventFiberDelegate, nullptr);
        if (msgloop_fiber_handle == nullptr || caller_fiber_handle == nullptr)
            throw std::runtime_error("fiber creation failed.");
    }
    
    // if we don't have any pending messages then switch to the 
    // message fiber to pump the message loop for a while.
    if (!impl::HasGlobalWindowMessage())
        SwitchToFiber(msgloop_fiber_handle);

    MSG m;
    if (!impl::GetGlobalWindowMessage(&m))
        return false;

    ev = native_event_t(m);
    return true;
}

void WaitEvent(native_event_t& ev)
{
    MSG m;

    while (!impl::HasGlobalWindowMessage())
    {
        // note that the GetMessage cleverly uses BOOL to
        // indicate ok, quit and error conditions. Hooray for 
        // true, false, file not found \o/
        if (GetMessage(&m, NULL, 0, 0) == -1)
            throw std::runtime_error("GetMessage failed");

        // translate virtual key messages into unicode characters
        // which are posted in WM_CHAR (UTF-16)
        // this works fine for BMP in the range 0x0000 - 0xD7FF, 0xE000 - 0xFFFF.
        // for values above 0xFFFF this is broken, but not an issue thus far.
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

    impl::GetGlobalWindowMessage(&m);
    ev = native_event_t(m);
}

std::pair<bitflag<Keymod>, Keysym> TranslateKeydownEvent(const native_event_t& key)
{
    const MSG& m = key;
    const uint_t native_keycode = (uint_t)m.wParam;

    std::pair<bitflag<Keymod>, Keysym> ret = {{}, Keysym::None};

    const auto it = std::find_if(std::begin(keymap), std::end(keymap),
        [=](const key_mapping& map)
        {
            return map.win == native_keycode;
        });

    if (it != std::end(keymap))
    {
        const short alt_key   = GetKeyState(VK_MENU);
        const short shift_key = GetKeyState(VK_SHIFT);
        const short ctrl_key  = GetKeyState(VK_CONTROL);

        if (native_keycode != VK_MENU && (alt_key & KEY_DOWN))
            ret.first |= Keymod::Alt;
        if (native_keycode != VK_SHIFT && (shift_key & KEY_DOWN))
            ret.first |= Keymod::Shift;
        if (native_keycode != VK_CONTROL && (ctrl_key & KEY_DOWN))
            ret.first |= Keymod::Control;
        ret.second = (*it).wdk;
    }
    else if (native_keycode == VK_CONTROL)
    {
        if (GetKeyState(VK_LCONTROL) & KEY_DOWN)
            ret.second = Keysym::ControlL;
        else if (GetKeyState(VK_RCONTROL) & KEY_DOWN)
            ret.second = Keysym::ControlR;
    }
    else if (native_keycode == VK_MENU)
    {
        if (GetKeyState(VK_LMENU) & KEY_DOWN)
            ret.second = Keysym::AltL;
      /*  else if (GetAsyncKeyState(VK_RMENU) & KEY_DOWN)
            sym = keysym::alt_R;*/
    }
    else if (native_keycode == VK_SHIFT)
    {
       if (GetKeyState(VK_LSHIFT) & KEY_DOWN)
            ret.second = Keysym::ShiftL;
       else if (GetKeyState(VK_RSHIFT) & KEY_DOWN)
            ret.second = Keysym::ShiftR;
    }

    return ret;
}

std::pair<bitflag<Keymod>, MouseButton> TranslateMouseButtonEvent(const native_event_t& btn)
{
    std::pair<bitflag<Keymod>, MouseButton> ret = {Keymod::None, MouseButton::None};

    const auto alt   = GetKeyState(VK_MENU);
    const auto shift = GetKeyState(VK_SHIFT);
    const auto ctrl  = GetKeyState(VK_CONTROL);
    if (alt & KEY_DOWN)
        ret.first |= Keymod::Alt;
    if (shift & KEY_DOWN)
        ret.first |= Keymod::Shift;
    if (ctrl & KEY_DOWN)
        ret.first |= Keymod::Control;

    const auto wparam  = btn.get().wParam;
    const auto message = btn.get().message;
    if (message == WM_MOUSEWHEEL)
    {
        const auto hi = HIWORD(wparam);
        if (hi > 0)
            ret.second = MouseButton::WheelScrollUp;
        else ret.second = MouseButton::WheelScrollDown;
    }
    else
    {
        switch (message)
        {
            case WM_LBUTTONUP:
            case WM_LBUTTONDOWN:
                ret.second = MouseButton::Left;
                break;
            case WM_MBUTTONUP:
            case WM_MBUTTONDOWN:
                ret.second = MouseButton::Wheel;
                break;
            case WM_RBUTTONUP:
            case WM_RBUTTONDOWN:
                ret.second = MouseButton::Right;
                break;
        }
        // other buttons as bitflags
        if (wparam & MK_LBUTTON)
            ret.second = MouseButton::Left;
        else if (wparam & MK_MBUTTON)
            ret.second = MouseButton::Wheel;
        else if (wparam & MK_RBUTTON)
            ret.second = MouseButton::Right;
    }
    return ret;
}

bool TestKeyDown(Keysym symbol)
{
    const UINT win    = find_keysym(symbol);
    const SHORT state = GetAsyncKeyState(win);

    return (state & KEY_DOWN) == KEY_DOWN;

}

bool TestKeyDown(uint_t keycode)
{
    assert(keycode);

    const SHORT state = GetAsyncKeyState(keycode);

    return (state & KEY_DOWN) == KEY_DOWN;
}

uint_t MapKeysymToNativeKeycode(Keysym symbol)
{
    const UINT win = find_keysym(symbol);

    return win;
}

} // wdk
