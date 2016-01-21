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

#include <boost/test/minimal.hpp>
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

#ifdef LINUX
#  include <unistd.h>
#endif

void unit_test_video_modes()
{
    {
        const wdk::videomode& original_mode = wdk::get_current_video_mode();
        BOOST_REQUIRE(original_mode.xres);
        BOOST_REQUIRE(original_mode.yres);
 
        const std::vector<wdk::videomode> modes = wdk::list_video_modes();

        BOOST_REQUIRE(find(modes.begin(), modes.end(), original_mode) != modes.end());

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
                const auto& e = wdk::get_event();
                if (e.identity() == wdk::native_event_t::type::system_resolution_change)
                    break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
            curmode = mode;
        }

        // restore to current
        set_video_mode(original_mode);
        if (curmode != original_mode)
        {
            while (true)
            {
                const auto& e = wdk::get_event();
                if (e.identity() == wdk::native_event_t::type::system_resolution_change)
                    break;
            }
        }
    }

    // try setting to an invalid mode
    {
        try 
        {
            wdk::set_video_mode(wdk::videomode(50, 50));
            BOOST_REQUIRE(!"failed to detect invalid video mode setting");
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
                const auto& e = wdk::get_event();
                if (e.identity() == wdk::native_event_t::type::system_resolution_change)
                    break;
            }
        }

        const wdk::videomode& now = wdk::get_current_video_mode();

        while (true)
        {
            const auto& e = wdk::get_event();
            if (e.identity() == wdk::native_event_t::type::system_resolution_change)
                break;
        }

        BOOST_REQUIRE(now == curmode);
    }

}

void unit_test_keyboard()
{
    for (int i = (int)wdk::keysym::backspace; i <= (int)wdk::keysym::escape; ++i)
    {
        const auto name = wdk::name((wdk::keysym)i);
        BOOST_REQUIRE(!name.empty());

        const auto code = keysym_to_keycode((wdk::keysym)i);
        BOOST_REQUIRE(code);

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


void unit_test_window()
{
    // test different sizes
    {
        wdk::window w;

        BOOST_REQUIRE(!w.exists());

        // create windows of various sizes, these should all be doable
        struct dim {
            wdk::uint_t width;
            wdk::uint_t height;
        };

        const dim sizes[] = {
            {600, 400},
            {555, 555},
            {1024, 728}
        };

        for (const auto& it : sizes)
        {
            w.create("unit-test", it.width, it.height, 0);
            BOOST_REQUIRE(w.surface_height() == it.height);
            BOOST_REQUIRE(w.surface_width() == it.width);
            BOOST_REQUIRE(w.exists());

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            const auto& min_size = w.min_size();
            const auto& max_size = w.max_size();

            BOOST_REQUIRE(min_size.first);
            BOOST_REQUIRE(min_size.second);
            BOOST_REQUIRE(max_size.first);
            BOOST_REQUIRE(max_size.second);

            w.set_size(min_size.first, min_size.second);
            w.sync_all_events();
            BOOST_REQUIRE(w.surface_width() == min_size.first);
            BOOST_REQUIRE(w.surface_height() == min_size.second);

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            w.set_size(max_size.first, max_size.second);
            w.sync_all_events();
            BOOST_REQUIRE(w.surface_width() == max_size.first);
            BOOST_REQUIRE(w.surface_height() == max_size.second);

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            w.destroy();
            BOOST_REQUIRE(!w.exists());
        }

    }

    // set to fullscreen
    {
        const wdk::videomode& current_mode = wdk::get_current_video_mode();

        wdk::window w;
        w.create("unit-test", 400, 400, 0);
        w.set_fullscreen(true);

        wdk::sync_events();
        BOOST_REQUIRE(w.surface_height() == current_mode.yres);
        BOOST_REQUIRE(w.surface_width() == current_mode.xres);

        w.destroy();

        w.create("unit-test", 400, 500, 0, false);
        const auto& min_size = w.min_size();
        const auto& max_size = w.max_size();

        BOOST_REQUIRE(min_size.first == 400);
        BOOST_REQUIRE(min_size.second == 500);
        BOOST_REQUIRE(max_size.first == 400);
        BOOST_REQUIRE(max_size.second == 500);
    }

    // test events
    {

        struct events : public wdk::window_listener 
        {
            void on_create(const wdk::window_event_create& create)
            {
                BOOST_REQUIRE(create.width == 600);
                BOOST_REQUIRE(create.height == 500);
                got_create = true;
            }
            void on_paint(const wdk::window_event_paint& paint)
            {
                got_paint = true;
            }
            void on_lost_focus(const wdk::window_event_focus&)
            {
                got_kill_focus = true;
            }
            void on_gain_focus(const wdk::window_event_focus&)
            {
                got_gain_focus = true;
            }
            void on_want_close(const wdk::window_event_want_close& close)
            {
            }
            void on_resize(const wdk::window_event_resize& resize)
            {
                BOOST_REQUIRE(resize.width == 200);
                BOOST_REQUIRE(resize.height == 300);
                got_resize = true;
            }

            bool got_create;
            bool got_paint;
            bool got_kill_focus;
            bool got_gain_focus;
            bool got_resize;
        };

        // create two windows and use top window to obscure the below window which gets us events
        wdk::window top;
        wdk::window below;

        events ev; 
        below.on_create = std::bind(&events::on_create, &ev, std::placeholders::_1);
        below.on_lost_focus = std::bind(&events::on_lost_focus, &ev, std::placeholders::_1);
        below.on_gain_focus = std::bind(&events::on_gain_focus, &ev, std::placeholders::_1);
        below.on_paint = std::bind(&events::on_paint, &ev, std::placeholders::_1);

        below.create("Below", 600, 500, 0);
        below.move(0, 0);
        below.sync_all_events();
        BOOST_REQUIRE(ev.got_create);

        ev.got_kill_focus = false;
        ev.got_gain_focus = false;
        ev.got_paint = false;

        top.create("Top", 800, 800, 0);
        top.move(0, 0);
        top.set_focus();

        below.sync_all_events();
        BOOST_REQUIRE(ev.got_kill_focus);

        top.destroy();

        below.sync_all_events();
        BOOST_REQUIRE(ev.got_gain_focus);
        BOOST_REQUIRE(ev.got_paint);

    }

}


int test_main(int, char*[])
{
    unit_test_video_modes();
    unit_test_keyboard();
    unit_test_window();
    return 0;
}