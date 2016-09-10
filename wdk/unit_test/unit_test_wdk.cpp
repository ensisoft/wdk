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


#include <wdk/system.h>
#include <wdk/videomode.h>
#include <wdk/modechange.h>
#include <wdk/window.h>
#include <wdk/window_events.h>
#include <wdk/window_listener.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <thread>
#include <chrono>

#include "test_minimal.h"

#define SYNC_POINT(x) std::this_thread::sleep_for(std::chrono::seconds(x));

#define SYNC_POINT_WIN(x, w) \
    do { \
        std::this_thread::sleep_for(std::chrono::seconds(x)); \
        wdk::native_event_t event; \
        while (wdk::peek_event(event)) \
            w.process_event(event); \
    } while (0)

void unit_test_video_modes()
{
    {
        const wdk::videomode& original_mode = wdk::get_current_video_mode();
        TEST_REQUIRE(original_mode.xres);
        TEST_REQUIRE(original_mode.yres);

        const std::vector<wdk::videomode> modes = wdk::list_video_modes();

        TEST_REQUIRE(find(modes.begin(), modes.end(), original_mode) != modes.end());

        wdk::videomode curmode = original_mode;

        for (auto& mode : modes)
        {
            if (mode == curmode)
                continue;
            std::cout << "Testing: " << mode << std::endl;
            wdk::set_video_mode(mode);

            // wait for a change event
            while (true)
            {
                wdk::native_event_t event;
                wdk::wait_event(event);
                if (event.identity() == wdk::native_event_t::type::system_resolution_change)
                    break;
            }

            SYNC_POINT(3);

            curmode = mode;
        }

        // restore to current
        set_video_mode(original_mode);
        if (curmode != original_mode)
        {
            while (true)
            {
                wdk::native_event_t event;
                wdk::wait_event(event);
                if (event.identity() == wdk::native_event_t::type::system_resolution_change)
                    break;
            }
        }
    }

    // try setting to an invalid mode
    {
        try
        {
            wdk::set_video_mode(wdk::videomode(50, 50));
            TEST_REQUIRE(!"failed to detect invalid video mode setting");
        }
        catch (const std::exception& e)
        {
            // success!
        }
    }

    // try restoring the mode
    {
        const wdk::videomode& curmode = wdk::get_current_video_mode();
        {
            wdk::modechange vidmode;
            vidmode.set(wdk::videomode(800, 600));
            while  (true)
            {
                wdk::native_event_t event;
                wdk::wait_event(event);
                if (event.identity() == wdk::native_event_t::type::system_resolution_change)
                    break;
            }
        }

        const wdk::videomode& now = wdk::get_current_video_mode();

        while (true)
        {
            wdk::native_event_t event;
            wdk::wait_event(event);
            if (event.identity() == wdk::native_event_t::type::system_resolution_change)
                break;
        }

        TEST_REQUIRE(now == curmode);
    }

}

void unit_test_keyboard()
{
    for (int i = (int)wdk::keysym::backspace; i <= (int)wdk::keysym::escape; ++i)
    {
        const auto name = wdk::name((wdk::keysym)i);
        TEST_REQUIRE(!name.empty());

        const auto code = keysym_to_keycode((wdk::keysym)i);
        TEST_REQUIRE(code);

        std::cout << "\rpress: " << name << std::endl;

        while (!wdk::test_key_down((wdk::keysym)i))
        {}

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "\rpress: " << name << std::endl;

        while (!wdk::test_key_down(code))
        {}

        std::cout << std::endl;
    }
}

void unit_test_window_functions()
{
    // test create/destroy/basic state
    {
        wdk::window w;
        TEST_REQUIRE(w.exists() == false);

        w.create("window", 400, 400, 0);
        TEST_REQUIRE(w.exists());
        TEST_REQUIRE(w.surface_height() == 400);
        TEST_REQUIRE(w.surface_width() == 400);
        TEST_REQUIRE(w.get_encoding() == wdk::window::encoding::utf8);
        TEST_REQUIRE(w.is_fullscreen() == false);

        w.set_size(400, 400);
        SYNC_POINT_WIN(1, w);        
        TEST_REQUIRE(w.surface_width() == 400);
        TEST_REQUIRE(w.surface_height() == 400);

        // set width bigger
        w.set_size(500, 400);
        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(w.surface_width() == 500);
        TEST_REQUIRE(w.surface_height() == 400);

        // set width smaller
        w.set_size(300, 400);
        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(w.surface_width() == 300);
        TEST_REQUIRE(w.surface_height() == 400);

        // set height bigger
        w.set_size(300, 500);
        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(w.surface_width() == 300);
        TEST_REQUIRE(w.surface_height() == 500);

        // set width smaller
        w.set_size(300, 300);
        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(w.surface_width() == 300);
        TEST_REQUIRE(w.surface_height() == 300);


        // set min/max size
        const auto& min_size = w.min_size();
        const auto& max_size = w.max_size();
        w.set_size(min_size.first, min_size.second);

        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(w.surface_width() == min_size.first);
        TEST_REQUIRE(w.surface_height() == min_size.second);

        w.set_size(max_size.first, max_size.second);

        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(w.surface_width() == max_size.first);
        TEST_REQUIRE(w.surface_height() == max_size.second);

        w.destroy();
        TEST_REQUIRE(w.exists() == false);
    }

    // test different sizes
    {
        wdk::window w;

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
            w.create("unit-test", it.width, it.height, 0);

            SYNC_POINT_WIN(1, w);

            TEST_REQUIRE(w.surface_height() == it.height);
            TEST_REQUIRE(w.surface_width() == it.width);


            w.destroy();
        }

    }

    // set to fullscreen
    {
        const wdk::videomode& current_mode = wdk::get_current_video_mode();

        wdk::window w;
        w.create("unit-test", 640, 400, 0);
        w.set_fullscreen(true);

        SYNC_POINT_WIN(1, w);

        TEST_REQUIRE(w.surface_height() == current_mode.yres);
        TEST_REQUIRE(w.surface_width() == current_mode.xres);
        TEST_REQUIRE(w.is_fullscreen() == true);

        w.set_fullscreen(false);

        SYNC_POINT_WIN(1, w);

        TEST_REQUIRE(w.surface_width() == 640);
        TEST_REQUIRE(w.surface_height() == 400);
        TEST_REQUIRE(w.is_fullscreen() == false);
        w.destroy();

    }
}



// test window create event.
// We should get create event when:
// - window is created.
void unit_test_window_create_event()
{
    const auto& screen = wdk::get_current_video_mode();
    TEST_REQUIRE(screen.xres >= 600);
    TEST_REQUIRE(screen.yres >= 500);

    bool got_create_event = false;

    wdk::window w;
    w.on_create = [&](const wdk::window_event_create& create) {
        TEST_REQUIRE(create.width == 600);
        TEST_REQUIRE(create.height == 500);
        TEST_REQUIRE(create.x >= 0);
        TEST_REQUIRE(create.y >= 0);
        TEST_REQUIRE(create.x + create.width <= screen.xres);
        TEST_REQUIRE(create.y + create.height <= screen.yres);
        got_create_event = true;
    };

    w.create("window", 600, 500, 0);

    SYNC_POINT_WIN(1, w);

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
        bool fired;
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;

        void clear() {
            fired = false;
            x = y = w = h = 0;
        }
    } paintEvent;

    paintEvent.clear();

    wdk::window w;
    w.on_paint = [&](const wdk::window_event_paint& paint) {
        paintEvent.x = paint.x;
        paintEvent.y = paint.y;
        paintEvent.w = paint.width;
        paintEvent.h = paint.height;
        paintEvent.fired = true;
    };
    w.create("window", 600,500, 0);
    SYNC_POINT_WIN(1, w);
    TEST_REQUIRE(paintEvent.fired == true);
    TEST_REQUIRE(paintEvent.x == 0);
    TEST_REQUIRE(paintEvent.y == 0);
    TEST_REQUIRE(paintEvent.h == 500);
    TEST_REQUIRE(paintEvent.w == 600);
    paintEvent.clear();

    w.destroy();
}

// test the resize event.
//
// we should get this event when the window has been resized.
// this includes: window creation, setting window size and
// having the user resize the window.
void unit_test_window_resize_event()
{
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

        wdk::window w;
        w.on_resize = [&](const wdk::window_event_resize& resize) {
            resizeEvent.fired = true;
            resizeEvent.w = resize.width;
            resizeEvent.h = resize.height;
        };

        w.create("try to resize the window", 600, 500, 0, true);
        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(resizeEvent.fired);
        TEST_REQUIRE(resizeEvent.w == 600);
        TEST_REQUIRE(resizeEvent.h == 500);
        resizeEvent.clear();

        w.set_size(300, 200);
        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(resizeEvent.fired);
        TEST_REQUIRE(resizeEvent.w == 300);
        TEST_REQUIRE(resizeEvent.h == 200);
        resizeEvent.clear();

        const auto& desktop = wdk::get_current_video_mode();

        w.set_fullscreen(true);
        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(resizeEvent.fired);
        TEST_REQUIRE(resizeEvent.w == desktop.xres);
        TEST_REQUIRE(resizeEvent.h == desktop.yres);
        resizeEvent.clear();


        // go back into windowed.
        w.set_fullscreen(false);
        SYNC_POINT_WIN(1, w);
        TEST_REQUIRE(resizeEvent.fired);
        TEST_REQUIRE(resizeEvent.w == 300);
        TEST_REQUIRE(resizeEvent.h == 200);
        resizeEvent.clear();



        // finally ask the user to resize the window.
        std::cout << "Please resize the window.\n";
        std::cout.flush();

        using clock = std::chrono::steady_clock;
        const auto now = clock::now();
        while (clock::now() - now < std::chrono::seconds(10))
        {
            const auto secs = std::chrono::duration_cast<std::chrono::seconds>(clock::now()-now);
            std::cout << "\r" << 10 - secs.count() << "s ...";
            std::cout.flush();

            wdk::native_event_t event;
            while (wdk::peek_event(event))
            {
                w.process_event(event);
            }
            if (resizeEvent.fired)
                break;
        }
        TEST_REQUIRE(resizeEvent.fired);
        TEST_REQUIRE(resizeEvent.w != 300);
        TEST_REQUIRE(resizeEvent.h != 200);
        resizeEvent.clear();

    }

}

// test the focus lost/gain event.
void unit_test_window_focus_event()
{
    bool oneHasFocus = false;
    bool twoHasFocus = false;

    // create two windows and swap between these two
    wdk::window one;
    one.on_lost_focus = [&](const wdk::window_event_focus&) {
        oneHasFocus = false;
    };
    one.on_gain_focus = [&](const wdk::window_event_focus&) {
        oneHasFocus = true;
    };

    wdk::window two;
    two.on_lost_focus = [&](const wdk::window_event_focus&) {
        twoHasFocus = false;
    };
    two.on_gain_focus = [&](const wdk::window_event_focus&) {
        twoHasFocus = true;
    };

    one.create("one", 100, 100, 0);
    one.set_focus();
    SYNC_POINT_WIN(1, one);
    TEST_REQUIRE(oneHasFocus == true);
    TEST_REQUIRE(twoHasFocus == false);

    two.create("two", 100, 100, 0);
    two.move(100, 100);
    two.set_focus();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    wdk::native_event_t event;
    while (wdk::peek_event(event))
    {
        one.process_event(event);
        two.process_event(event);
    }

    TEST_REQUIRE(twoHasFocus == true);
    TEST_REQUIRE(oneHasFocus == false);

}

void unit_test_window_close_event()
{
    wdk::window win;
    win.create("click window close 'x' or alt+f4", 500, 500, 0);

    bool want_close = false;

    win.on_want_close = [&](const wdk::window_event_want_close&) {
        want_close = true;
    };

    std::cout << "Please click on the X\n";
    for (int i=0; i<10; ++i)
    {
        std::cout << "\r" << 10 - i << "s ...";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    SYNC_POINT_WIN(1, win);

    TEST_REQUIRE(want_close);

}

void unit_test_window_key_event()
{
    wdk::window win;
    win.set_encoding(wdk::window::encoding::ascii);
    win.create("press 'A' key on the window", 500, 500, 0);

    bool on_key_down = false;
    bool on_key_up   = false;
    bool on_char     = false;

    win.on_keydown = [&](const wdk::window_event_keydown& key) {
        TEST_REQUIRE(key.symbol == wdk::keysym::key_A);
        on_key_down = true;
    };
    win.on_keyup = [&](const wdk::window_event_keyup& key) {

        // there's a weird problem on X11 here that when this test case is
        // run something somewhere generates an Enter press in the input queue.
        // Current theory is that this Enter is the KeyRelease event that was
        // generated for the terminal (xterm). In other words when the command
        // to run this test application is executed the terminal reacts to
        // the X's KeyPress event and runs the this application which grabs the
        // I.e. the terminal responds to X's KeyPress event and either the terminal
        // or X server then itself sends the KeyRelease event to the newly launched
        // application.
        if (key.symbol == wdk::keysym::enter && !on_key_up)
            return;

        TEST_REQUIRE(key.symbol == wdk::keysym::key_A);
        on_key_up = true;
    };
    win.on_char = [&](const wdk::window_event_char& c) {
        TEST_REQUIRE(c.ascii == 'a');
        on_char = true;
    };

    std::cout << "Please press 'A' key on the window\n";
    std::cout.flush();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    using clock = std::chrono::steady_clock;
    auto now = clock::now();
    while (clock::now() - now < std::chrono::seconds(10))
    {
        const auto secs = std::chrono::duration_cast<std::chrono::seconds>(clock::now() - now);
        std::cout << "\r" << 10 - secs.count() << "s ...";
        std::cout.flush();

        wdk::native_event_t event;
        while (wdk::peek_event(event))
        {
            win.process_event(event);
        }
        if (on_key_down && on_key_up && on_char)
            break;
    }

    TEST_REQUIRE(on_key_down);
    TEST_REQUIRE(on_key_up);
    TEST_REQUIRE(on_char);
}

// test mouse events
void unit_test_window_mouse_events()
{
    wdk::window win;
    win.create("move mouse and click left button", 500, 500, 0);

    bool on_mouse_move    = false;
    bool on_mouse_press   = false;
    bool on_mouse_release = false;

    win.on_mouse_move = [&](const wdk::window_event_mouse_move& mickey) {
        on_mouse_move = true;
    };
    win.on_mouse_press = [&](const wdk::window_event_mouse_press& mickey) {
        TEST_REQUIRE(mickey.btn == wdk::button::left);
        on_mouse_press = true;
    };
    win.on_mouse_release = [&](const wdk::window_event_mouse_release& mickey) {
        TEST_REQUIRE(mickey.btn == wdk::button::left);
        on_mouse_release = true;
    };

    std::cout << "Please press left mouse button on the window\n";
    std::cout.flush();

    using clock = std::chrono::steady_clock;
    auto now = clock::now();
    while (clock::now() - now < std::chrono::seconds(10))
    {
        const auto secs = std::chrono::duration_cast<std::chrono::seconds>(clock::now() - now);
        std::cout << "\r" << 10 - secs.count() << "s ...";
        std::cout.flush();

        wdk::native_event_t event;
        while (wdk::peek_event(event))
        {
            win.process_event(event);
        }
        if (on_mouse_move && on_mouse_press && on_mouse_release)
            break;
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
    unit_test_window_key_event();
    unit_test_window_mouse_events();
    return 0;
}
