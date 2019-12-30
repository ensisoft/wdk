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
#include <iostream>
#include <iterator>
#include <thread>
#include <chrono>

#include "wdk/system.h"
#include "wdk/videomode.h"
#include "wdk/modechange.h"
#include "wdk/window.h"
#include "wdk/events.h"
#include "wdk/window_listener.h"

#include "test_minimal.h"

bool WaitVideoModeChange()
{
    // pump the application message loop for a while expecting to 
    // find a resolution change message.
    // note that displays will take some time before the resolution
    // change is actually visible to the user. i.e. while we might
    // have received the message already the user might not yet see
    // the change reflected on the display.
    for (int i=0; i<3; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        wdk::native_event_t event;
        while (wdk::PeekEvent(event))
        {
            if (event.identity() == wdk::native_event_t::type::system_resolution_change) 
                return true;
        }
    }
    return false;
}

void ProcessWindowEvents(wdk::Window& win)
{
    wdk::native_event_t event;
    while (wdk::PeekEvent(event))
        win.ProcessEvent(event);
}

void ProcessWindowEvents(wdk::Window& win, unsigned secs)
{
    std::this_thread::sleep_for(std::chrono::seconds(secs));
    wdk::native_event_t event;
    while (wdk::PeekEvent(event))
        win.ProcessEvent(event);
}

void unit_test_video_modes()
{
    // test enumerating supported video modes and setting the mode.
    {
        const auto& mode  = wdk::GetCurrentVideoMode();
        const auto& modes = wdk::ListVideoModes();        
        TEST_REQUIRE(mode.xres);
        TEST_REQUIRE(mode.yres);
        TEST_REQUIRE(find(modes.begin(), modes.end(), mode) != modes.end());

        for (auto& m : modes)
        {
            if (m == mode)
                continue;
            
            wdk::SetVideoMode(m);
            TEST_REQUIRE(WaitVideoModeChange());
        }
        // restore to original mode.
        wdk::SetVideoMode(mode);
        TEST_REQUIRE(WaitVideoModeChange());
    }

    // test setting video mode with wait to see the change.
    {
        const auto& modes = wdk::ListVideoModes();

        const wdk::VideoMode XGA = {1024, 768};
        const auto ret = std::find(std::begin(modes), std::end(modes), XGA);
        if (ret != std::end(modes)) 
        {
            wdk::TemporaryVideoModeChange change(XGA);
            TEST_REQUIRE(WaitVideoModeChange());
            std::this_thread::sleep_for(std::chrono::seconds(4));
        }
    }

    // try setting to an invalid mode, expect to fail
    {
        try
        {
            wdk::SetVideoMode(wdk::VideoMode(50, 50));
            TEST_REQUIRE(!"failed to detect invalid video mode setting");
        }
        catch (const std::exception& e)
        {
            // success!
        }
    }
}

void unit_test_keyboard()
{
    for (int i = (int)wdk::Keysym::Backspace; i <= (int)wdk::Keysym::Escape; ++i)
    {
        const auto name = wdk::ToString((wdk::Keysym)i);
        TEST_REQUIRE(!name.empty());

        const auto code = MapKeysymToNativeKeycode((wdk::Keysym)i);
        TEST_REQUIRE(code);

        std::cout << "\rpress: " << name << std::endl;

        while (!wdk::TestKeyDown((wdk::Keysym)i))
        {}

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "\rpress: " << name << std::endl;

        while (!wdk::TestKeyDown(code))
        {}

        std::cout << std::endl;
    }
}

void unit_test_window_functions()
{
    // test create/destroy/basic state
    {
        wdk::Window w;
        TEST_REQUIRE(w.DoesExist() == false);

        w.Create("window", 400, 400, 0);
        TEST_REQUIRE(w.DoesExist());
        TEST_REQUIRE(w.GetSurfaceHeight() == 400);
        TEST_REQUIRE(w.GetSurfaceWidth() == 400);
        TEST_REQUIRE(w.GetEncoding() == wdk::Window::Encoding::UTF8);
        TEST_REQUIRE(w.IsFullscreen() == false);

        w.SetSize(400, 400);
        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceWidth() == 400);
        TEST_REQUIRE(w.GetSurfaceHeight() == 400);

        // set width bigger
        w.SetSize(500, 400);
        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceWidth() == 500);
        TEST_REQUIRE(w.GetSurfaceHeight() == 400);

        // set width smaller
        w.SetSize(300, 400);
        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceWidth() == 300);
        TEST_REQUIRE(w.GetSurfaceHeight() == 400);

        // set height bigger
        w.SetSize(300, 500);
        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceWidth() == 300);
        TEST_REQUIRE(w.GetSurfaceHeight() == 500);

        // set width smaller
        w.SetSize(300, 300);
        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceWidth() == 300);
        TEST_REQUIRE(w.GetSurfaceHeight() == 300);


        // set min/max size
        const auto& min_size = w.GetMinSize();
        const auto& max_size = w.GetMaxSize();
        w.SetSize(min_size.first, min_size.second);

        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceWidth() == min_size.first);
        TEST_REQUIRE(w.GetSurfaceHeight() == min_size.second);

        w.SetSize(max_size.first, max_size.second);

        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceWidth() == max_size.first);
        TEST_REQUIRE(w.GetSurfaceHeight() == max_size.second);

        w.Destroy();
        TEST_REQUIRE(w.DoesExist() == false);
    }

    // test different sizes
    {
        // create windows of various sizes, these should all be doable
        struct dim {
            wdk::uint_t width;
            wdk::uint_t height;
        };

        const dim sizes[] = {
            {150, 150},
            {600, 400},
            {555, 555},
            {1024, 768},
            {1600, 1000}
        };
        for (const auto& it : sizes)
        {
            wdk::Window w;
            w.Create("unit-test", it.width, it.height, 0);
            ProcessWindowEvents(w, 1);
            TEST_REQUIRE(w.GetSurfaceHeight() == it.height);
            TEST_REQUIRE(w.GetSurfaceWidth() == it.width);
        }
    }

    // set to fullscreen
    {
        const wdk::VideoMode& current_mode = wdk::GetCurrentVideoMode();

        wdk::Window w;
        w.Create("unit-test", 640, 400, 0);
        w.SetFullscreen(true);

        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceHeight() == current_mode.yres);
        TEST_REQUIRE(w.GetSurfaceWidth() == current_mode.xres);
        TEST_REQUIRE(w.IsFullscreen() == true);

        w.SetFullscreen(false);
        ProcessWindowEvents(w, 1);
        TEST_REQUIRE(w.GetSurfaceWidth() == 640);
        TEST_REQUIRE(w.GetSurfaceHeight() == 400);
        TEST_REQUIRE(w.IsFullscreen() == false);
        w.Destroy();

    }
}

// test window create event.
// We should get create event when:
// - window is created.
void unit_test_window_create_event()
{
    const auto& screen = wdk::GetCurrentVideoMode();
    TEST_REQUIRE(screen.xres >= 600);
    TEST_REQUIRE(screen.yres >= 500);

    bool got_create_event = false;

    wdk::Window w;
    w.on_create = [&](const wdk::WindowEventCreate& create) {
        TEST_REQUIRE(create.width == 600);
        TEST_REQUIRE(create.height == 500);
        TEST_REQUIRE(create.x >= 0);
        TEST_REQUIRE(create.y >= 0);
        TEST_REQUIRE(create.x + create.width <= screen.xres);
        TEST_REQUIRE(create.y + create.height <= screen.yres);
        got_create_event = true;
    };

    w.Create("window", 600, 500, 0);

    ProcessWindowEvents(w, 1);
    TEST_REQUIRE(got_create_event == true);
}

// test paint event.
//
// this specific event and it's invalid region parameters
// depend on the underlying implementation and may not be portably
// tested across all implementations, however we're going to assume
// that at least creating the window is followed by a request to paint
// the whole window area.
void unit_test_window_paint_event()
{
    struct PaintEvent {
        bool fired = false;
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
    } paintEvent;

    wdk::Window w;
    w.on_paint = [&](const wdk::WindowEventPaint& paint) {
        paintEvent.x = paint.x;
        paintEvent.y = paint.y;
        paintEvent.w = paint.width;
        paintEvent.h = paint.height;
        paintEvent.fired = true;
    };
    w.Create("window", 600,500, 0);
    ProcessWindowEvents(w, 1);
    TEST_REQUIRE(paintEvent.fired == true);
    TEST_REQUIRE(paintEvent.x == 0);
    TEST_REQUIRE(paintEvent.y == 0);
    TEST_REQUIRE(paintEvent.h == 500);
    TEST_REQUIRE(paintEvent.w == 600);
    
    paintEvent = PaintEvent{};

    w.Invalidate();
    ProcessWindowEvents(w, 1);
    TEST_REQUIRE(paintEvent.fired == true);
    TEST_REQUIRE(paintEvent.x == 0);
    TEST_REQUIRE(paintEvent.y == 0);
    TEST_REQUIRE(paintEvent.h == 500);
    TEST_REQUIRE(paintEvent.w == 600);

    const auto& mode = wdk::GetCurrentVideoMode();

    paintEvent = PaintEvent{};
    w.SetFullscreen(true);
    ProcessWindowEvents(w, 1);
    TEST_REQUIRE(paintEvent.fired == true);
    // todo: these expectations do not currently hold on X11
    //TEST_REQUIRE(paintEvent.x == 0);
    //TEST_REQUIRE(paintEvent.y == 0);
    //TEST_REQUIRE(paintEvent.h == mode.yres);
    //TEST_REQUIRE(paintEvent.w == mode.xres);

    w.Destroy();
}

// test the resize event.
//
// we should get this event when the window has been resized.
// this includes: window creation, setting window size and
// having the user resize the window.
void unit_test_window_resize_event()
{
    
    struct ResizeEvent {
        bool fired;
        int w;
        int h;
        void clear() {
            fired = false;
            w = h = 0;
        }
    } resizeEvent;

    resizeEvent.clear();

    wdk::Window w;
    w.on_resize = [&](const wdk::WindowEventResize& resize) {
        resizeEvent.fired = true;
        resizeEvent.w = resize.width;
        resizeEvent.h = resize.height;
    };

    w.Create("try to resize the window", 600, 500, 0, true);
    ProcessWindowEvents(w, 1);
    TEST_REQUIRE(resizeEvent.fired);
    TEST_REQUIRE(resizeEvent.w == 600);
    TEST_REQUIRE(resizeEvent.h == 500);
    resizeEvent.clear();

    w.SetSize(300, 200);
    ProcessWindowEvents(w, 1);
    TEST_REQUIRE(resizeEvent.fired);
    TEST_REQUIRE(resizeEvent.w == 300);
    TEST_REQUIRE(resizeEvent.h == 200);
    resizeEvent.clear();

    const auto& desktop = wdk::GetCurrentVideoMode();

    w.SetFullscreen(true);
    ProcessWindowEvents(w, 1);
    TEST_REQUIRE(resizeEvent.fired);
    TEST_REQUIRE(resizeEvent.w == desktop.xres);
    TEST_REQUIRE(resizeEvent.h == desktop.yres);
    resizeEvent.clear();

    // go back into windowed.
    w.SetFullscreen(false);
    ProcessWindowEvents(w, 1);
    TEST_REQUIRE(resizeEvent.fired);
    TEST_REQUIRE(resizeEvent.w == 300);
    TEST_REQUIRE(resizeEvent.h == 200);
    resizeEvent.clear();

    // finally ask the user to resize the window.
    std::cout << "\nPlease resize the window.\n";
    std::cout.flush();

    for (int i=0; i<10; ++i)
    {
        std::cout << "\r" << 10 - i << "s ...";
        std::cout.flush();
        ProcessWindowEvents(w);
        if (resizeEvent.fired)
            break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    TEST_REQUIRE(resizeEvent.fired);
    TEST_REQUIRE(resizeEvent.w != 300);
    TEST_REQUIRE(resizeEvent.h != 200);
    resizeEvent.clear();
}

// test the focus lost/gain event.
void unit_test_window_focus_event()
{
    bool oneHasFocus = false;
    bool twoHasFocus = false;

    // create two windows and swap between these two
    wdk::Window one;
    one.on_lost_focus = [&](const wdk::WindowEventFocus&) {
        oneHasFocus = false;
    };
    one.on_gain_focus = [&](const wdk::WindowEventFocus&) {
        oneHasFocus = true;
    };

    wdk::Window two;
    two.on_lost_focus = [&](const wdk::WindowEventFocus&) {
        twoHasFocus = false;
    };
    two.on_gain_focus = [&](const wdk::WindowEventFocus&) {
        twoHasFocus = true;
    };

    one.Create("one", 100, 100, 0);
    one.SetFocus();
    ProcessWindowEvents(one, 1);
    TEST_REQUIRE(oneHasFocus == true);
    TEST_REQUIRE(twoHasFocus == false);

    two.Create("two", 100, 100, 0);
    two.Move(100, 100);
    two.SetFocus();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    wdk::native_event_t event;
    while (wdk::PeekEvent(event))
    {
        one.ProcessEvent(event);
        two.ProcessEvent(event);
    }

    TEST_REQUIRE(twoHasFocus == true);
    TEST_REQUIRE(oneHasFocus == false);

}

void unit_test_window_close_event()
{
    wdk::Window win;
    win.Create("click window close 'x' or alt+f4", 500, 500, 0);

    bool want_close = false;

    win.on_want_close = [&](const wdk::WindowEventWantClose&) {
        want_close = true;
    };

    std::cout << "\nPlease click on the X\n";
    std::cout.flush();
    for (int i=0; i<10; ++i)
    {
        std::cout << "\r" << 10 - i << "s ...";
        std::cout.flush();
        ProcessWindowEvents(win);
        if (want_close)
            break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    TEST_REQUIRE(want_close);
}

void unit_test_window_key_event(wdk::Keysym expected_key, char expected_character)
{
    wdk::Window win;
    win.SetEncoding(wdk::Window::Encoding::ASCII);
    win.Create("Press key: " + ToString(expected_key), 500, 500, 0);

    bool on_key_down = false;
    bool on_key_up   = false;
    char received_character = 0;

    win.on_keydown = [&](const wdk::WindowEventKeydown& key) {
        TEST_REQUIRE(key.symbol == expected_key);
        on_key_down = true;
    };
    win.on_keyup = [&](const wdk::WindowEventKeyup& key) {

        // there's a weird problem on X11 here that when this test case is
        // run something somewhere generates an Enter press in the input queue.
        // Current theory is that this Enter is the KeyRelease event that was
        // generated for the terminal (xterm). In other words when the command
        // to run this test application is executed the terminal reacts to
        // the X's KeyPress event and runs the this application which grabs the
        // I.e. the terminal responds to X's KeyPress event and either the terminal
        // or X server then itself sends the KeyRelease event to the newly launched
        // application.
        if (key.symbol == wdk::Keysym::Enter && !on_key_up)
            return;

        TEST_REQUIRE(key.symbol == expected_key);
        on_key_up = true;
    };
    win.on_char = [&](const wdk::WindowEventChar& c) {
        received_character = c.ascii;
    };

    std::cout << "\nPlease press key: " << ToString(expected_key);
    std::cout.flush();

    for (int i=0; i<10; ++i) 
    {
        std::cout << "\r" << 10 - i << "s ...";
        std::cout.flush();
        ProcessWindowEvents(win);
        if (on_key_down && on_key_up && !expected_character)
            break;
        else if (on_key_down && on_key_up && received_character)
            break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    TEST_REQUIRE(on_key_down);
    TEST_REQUIRE(on_key_up);
    TEST_REQUIRE(expected_character == received_character);
}

// test mouse events
void unit_test_window_mouse_events(wdk::MouseButton expected_button)
{
    wdk::Window win;
    win.Create("Move mouse and click button: " + ToString(expected_button), 
        500, 500, 0);

    bool on_mouse_move    = false;
    bool on_mouse_press   = false;
    bool on_mouse_release = false;

    win.on_mouse_move = [&](const wdk::WindowEventMouseMove& mickey) {
        on_mouse_move = true;
    };
    win.on_mouse_press = [&](const wdk::WindowEventMousePress& mickey) {
        TEST_REQUIRE(mickey.btn == expected_button);
        on_mouse_press = true;
    };
    win.on_mouse_release = [&](const wdk::WindowEventMouseRelease& mickey) {
        TEST_REQUIRE(mickey.btn == expected_button);
        on_mouse_release = true;
    };

    std::cout << "\nPlease click mouse button: " << ToString(expected_button);
    std::cout.flush();

    for (int i=0; i<10; ++i)
    {
        std::cout << "\r" << 10 - i << "s ...";
        std::cout.flush();
        ProcessWindowEvents(win);
        if (on_mouse_move && on_mouse_press && on_mouse_release)
            break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    TEST_REQUIRE(on_mouse_move);
    TEST_REQUIRE(on_mouse_press);
    TEST_REQUIRE(on_mouse_release);
}

int test_main(int, char*[])
{
    unit_test_video_modes();
    unit_test_keyboard();
    unit_test_window_functions();
    unit_test_window_create_event();
    unit_test_window_paint_event();
    unit_test_window_resize_event();
    unit_test_window_focus_event();
    unit_test_window_close_event();
    unit_test_window_key_event(wdk::Keysym::KeyA, 'a');
    unit_test_window_key_event(wdk::Keysym::KeyZ, 'z');    
    unit_test_window_key_event(wdk::Keysym::Key0, '0');        
    unit_test_window_key_event(wdk::Keysym::F1, 0);
    unit_test_window_key_event(wdk::Keysym::ShiftL, 0);
    unit_test_window_key_event(wdk::Keysym::ArrowDown, 0);
    unit_test_window_mouse_events(wdk::MouseButton::Left);
    unit_test_window_mouse_events(wdk::MouseButton::Right);
    unit_test_window_mouse_events(wdk::MouseButton::Wheel);
    return 0;
}
