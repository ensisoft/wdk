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
#include <wdk/bitflag.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cassert>
#include <cstring>



void handle_window_keydown(const wdk::window_event_keydown& key, wdk::Window& win)
{
    printf("Keydown event: ");

    if (key.modifiers.test(wdk::KeyModifier::Shift))
        printf("Shift+");
    if (key.modifiers.test(wdk::KeyModifier::Control))
        printf("Ctrl+");
    if (key.modifiers.test(wdk::KeyModifier::Alt))
        printf("Alt+");

    const auto& str = wdk::GetKeySymbolName(key.symbol);
    printf("%s\n", str.c_str());

    if (key.symbol == wdk::KeySymbol::Escape)
        win.Destroy();
    else if (key.symbol == wdk::KeySymbol::Space)
        win.SetToFullscreen(!win.IsFullscreen());
}

void handle_window_keyup(const wdk::window_event_keyup& key, wdk::Window& win)
{
    printf("keyup event: ");

    if (key.modifiers.test(wdk::KeyModifier::Shift))
        printf("Shift+");
    if (key.modifiers.test(wdk::KeyModifier::Control))
        printf("Ctrl+");
    if (key.modifiers.test(wdk::KeyModifier::Alt))
        printf("Alt+");

    const auto& str = wdk::GetKeySymbolName(key.symbol);
    printf("%s\n", str.c_str());

}

void handle_window_create(const wdk::window_event_create& create)
{
    printf("Window create: @%d,%d, %d x %d\n", create.x, create.y, create.width, create.height);
}

void handle_window_want_close(const wdk::window_event_want_close& want_close)
{
    printf("Window want close:\n");
}

void handle_window_paint(const wdk::window_event_paint& paint)
{
    printf("Window paint: @%d,%d, %dx%d\n", paint.x, paint.y, paint.width, paint.height);
}

void handle_window_resize(const wdk::window_event_resize& resize)
{
    printf("Window resize: %d x %d\n", resize.width, resize.height);
}


void handle_window_gain_focus(const wdk::window_event_focus& focus)
{
    printf("Window got focus\n");
}

void handle_window_lost_focus(const wdk::window_event_focus& focus)
{
    printf("Window lost focus\n");
}

void handle_window_char(const wdk::window_event_char& uchar, wdk::Window& win)
{
    const auto e = win.GetCharcterEncoding();

    if (e == wdk::Window::encoding::ascii)
        printf("ASCII char event: %c\n", uchar.ascii);
    else if (e == wdk::Window::encoding::utf8)
       printf("UTF8 char event: \"%s\"\n", uchar.utf8);

}

void handle_window_mouse_move(const wdk::window_event_mouse_move& mickey)
{
    printf("Mouse move (win) %d,%d (root) %d,%d\n",
        mickey.window_x, mickey.window_y,
        mickey.global_x, mickey.global_x);
}

void handle_window_mouse_press(const wdk::window_event_mouse_press& mickey)
{
    printf("Mouse press: ");

    if (mickey.modifiers.test(wdk::KeyModifier::Shift))
        printf("Shift+");
    if (mickey.modifiers.test(wdk::KeyModifier::Control))
        printf("Ctrl+");
    if (mickey.modifiers.test(wdk::KeyModifier::Alt))
        printf("Alt+");

    const auto& str = wdk::GetMouseButtonName(mickey.btn);
    printf(" '%s'", str.c_str());

    printf(" (win) %d,%d (root) %d,%d\n",
        mickey.window_x, mickey.window_y,
        mickey.global_x, mickey.global_y);
}

struct cmdline {
    bool   print_help;
    bool   fullscreen;
    bool   listmodes;
    bool   wnd_border;
    bool   wnd_resize;
    int    surface_width;
    int    surface_height;
    wdk::Window::encoding encoding;
    wdk::VideoMode mode;
};

bool parse_cmdline(int argc, char* argv[], cmdline& cmd)
{
    for (int i=1; i<argc; ++i)
    {
        const char* name = argv[i];

        if (!strcmp(name, "--help"))
            cmd.print_help = true;
        else if (!strcmp(name, "--fullscreen"))
            cmd.fullscreen = true;
        else if (!strcmp(name, "--list-modes"))
            cmd.listmodes = true;
        else if (!strcmp(name, "--wnd-no-border"))
            cmd.wnd_border = false;
        else if (!strcmp(name, "--wnd-no-resize"))
            cmd.wnd_resize = false;
        else
        {
            if (!(i + 1 < argc))
                return false;

            if (!strcmp(name, "--encoding"))
            {
                const char* value = argv[++i];
                if (!strcmp(value, "ascii"))
                    cmd.encoding = wdk::Window::encoding::ascii;
                else if (!strcmp(value, "utf8"))
                    cmd.encoding = wdk::Window::encoding::utf8;
                else if (!strcmp(value, "ucs2"))
                    cmd.encoding = wdk::Window::encoding::ucs2;
            }
            else if (!strcmp(name, "--videomode"))
            {
                const char* value = argv[++i];
                int xres, yres;
                sscanf(value, "%dx%d", &xres, &yres);
                cmd.mode.xres = xres;
                cmd.mode.yres = yres;
            }
            else if (!strcmp(name, "--wnd-width"))
            {
                long value = atoi(argv[++i]);
                cmd.surface_width = value;
            }
            else if (!strcmp(name, "--wnd-height"))
            {
                long value = atoi(argv[++i]);
                cmd.surface_height = value;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
    cmdline cmd;
    cmd.print_help     = false;
    cmd.fullscreen     = false;
    cmd.listmodes      = false;
    cmd.wnd_border     = true;
    cmd.wnd_resize     = true;
    cmd.surface_width  = 640;
    cmd.surface_height = 480;
    cmd.encoding       = wdk::Window::encoding::ascii;

    if (!parse_cmdline(argc, argv, cmd))
    {
        std::cerr << "Incorrect command line\n";
        return 1;
    }
    else if (cmd.print_help)
    {
        std::cout
        << "\n"
        << "--help\t\t\tPrint this help\n"
        << "--fullscreen\t\tChange into fullscreen\n"
        << "--list-modes\t\tList available video modes\n"
        << "--wnd-no-border\t\tDisable window border\n"
        << "--wnd-no-resize\t\tDisable window resizing\n"
        << "--wnd-no-move\t\tDisable window moving\n"
        << "--wnd-width\t\tWindow width\n"
        << "--wnd-height\t\tWindow height\n"
        << "--video-mode\t\tVideo mode\n\n";
        return 0;
    }

    if (cmd.listmodes)
    {
        auto modes = wdk::ListAvailableVideoModes();

        std::sort(modes.begin(), modes.end(), std::greater<wdk::VideoMode>());
        std::copy(modes.begin(), modes.end(), std::ostream_iterator<wdk::VideoMode>(std::cout, "\n"));
        return 0;
    }

    wdk::modechange vidmode;

    if (!cmd.mode.IsNull())
        vidmode.set(cmd.mode);

    wdk::Window win;
    win.on_create     = handle_window_create;
    win.on_lost_focus = handle_window_lost_focus;
    win.on_gain_focus = handle_window_gain_focus;
    win.on_resize     = handle_window_resize;
    win.on_want_close = handle_window_want_close;
    win.on_paint      = handle_window_paint;
    win.on_mouse_move = handle_window_mouse_move;
    win.on_mouse_press = handle_window_mouse_press;
    win.on_char       = std::bind(handle_window_char, std::placeholders::_1, std::ref(win));
    win.on_keydown    = std::bind(handle_window_keydown, std::placeholders::_1, std::ref(win));
    win.on_keyup      = std::bind(handle_window_keyup, std::placeholders::_1, std::ref(win));

    win.Create("Wdk",
        cmd.surface_width,
        cmd.surface_height,
        0,
        cmd.wnd_resize,
        cmd.wnd_border);

    win.SetCharacterEncoding(cmd.encoding);
    win.SetToFullscreen(cmd.fullscreen);


    while (win.DoesExist())
    {
        wdk::native_event_t event;
        wdk::WaitEvent(event);
        win.ProcessWindowEvent(event);
    }

    return 0;
}


