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

#include <wdk/display.h>
#include <wdk/window.h>
#include <wdk/events.h>
#include <wdk/keyboard.h>
#include <wdk/event.h>
#include <wdk/ime.h>
#include <wdk/dispatch.h>
#include <iostream>
#include <iterator>
#include <cassert>
#include <cstring>

void handle_keydown(const wdk::keyboard_event_keydown& key, wdk::keyboard& kb, wdk::window& win)
{
    printf("Keydown event: ");

    if ((key.modifiers & wdk::keymod::shift) == wdk::keymod::shift)
        printf("Shift+");
    if ((key.modifiers & wdk::keymod::control) == wdk::keymod::control)
        printf("Ctrl+");
    if ((key.modifiers & wdk::keymod::alt) == wdk::keymod::alt)
        printf("Alt+");

    const std::string& sym = kb.name(key.symbol);
    printf("%s\n", sym.c_str());

    if (key.symbol == wdk::keysym::escape)
        win.close();

}

void handle_keyup(const wdk::keyboard_event_keyup& key)
{
    printf("Keyup event: (%d) (%d)\n", (int)key.symbol, (int)key.modifiers);
}

void handle_window_create(const wdk::window_event_create& create)
{
    printf("Window create: @%d,%d, %d x %d\n", create.x, create.y, create.width, create.height);
}

void handle_window_query_close(wdk::window_event_query_close& query_close)
{
    printf("Window query close:\n");
}

void handle_window_paint(const wdk::window_event_paint& paint)
{
    printf("Window paint: @%d,%d, %dx%d\n", paint.x, paint.y, paint.width, paint.height);
}

void handle_window_resize(const wdk::window_event_resize& resize)
{
    printf("Window resize: %d x %d\n", resize.width, resize.height);
}

void handle_window_destroy(const wdk::window_event_destroy& destroy)
{
   printf("Window destroyed\n");
}

void handle_window_gain_focus(const wdk::window_event_focus& focus)
{
    printf("Window got focus\n");
}

void handle_window_lost_focus(const wdk::window_event_focus& focus)
{
    printf("Window lost focus\n");
}

void handle_character(const wdk::ime_event_char& ime_char, const wdk::ime& im)
{
    using namespace wdk;
    ime::output out = im.get_output();

    if (out == ime::output::utf8)
        printf("UTF8 char event: \"%s\"\n", ime_char.utf8);
    else if (out == ime::output::ascii)
        printf("ASCII char event: %c\n", ime_char.ascii);
}

struct cmdline {
    bool   print_help;
    bool   fullscreen;
    bool   listmodes;
    bool   wnd_border;
    bool   wnd_resize;
    int    surface_width;
    int    surface_height;
    wdk::ime::output ime_output;
    wdk::native_vmode_t videomode;
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

            if (!strcmp(name, "--ime-output"))
            {
                const char* value = argv[++i];
                if (!strcmp(value, "ascii"))
                    cmd.ime_output = wdk::ime::output::ascii;
                else if (!strcmp(value, "utf8"))
                    cmd.ime_output = wdk::ime::output::utf8;
                else if (!strcmp(value, "ucs2"))
                    cmd.ime_output = wdk::ime::output::ucs2;
            }
            else
            {
                long value = atoi(argv[++i]);
                if (!strcmp(name, "--wnd-width"))
                    cmd.surface_width = value;
                else if (!strcmp(name, "--wnd-height"))
                    cmd.surface_height = value;
                else if (!strcmp(name, "--video-mode"))
                    cmd.videomode = static_cast<wdk::native_vmode_t>(value);
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
    cmd.videomode      = wdk::DEFAULT_VIDEO_MODE;
    cmd.ime_output     = wdk::ime::output::ascii;

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

    // connect to display server
    wdk::display disp;

    if (cmd.listmodes)
    {
        const auto modes = disp.list_video_modes();

        std::copy(modes.rbegin(), modes.rend(), std::ostream_iterator<wdk::videomode>(std::cout, "\n"));
        return 0;        
    }

    if (cmd.videomode != wdk::DEFAULT_VIDEO_MODE)
        disp.set_video_mode(cmd.videomode);

    wdk::window_style style = wdk::window_style::none;
    if (cmd.wnd_border)
        style |= wdk::window_style::border;
    if (cmd.wnd_resize)
        style |= wdk::window_style::resize;

    // window
    wdk::window win(disp);
    win.event_create      = handle_window_create;
    win.event_query_close = handle_window_query_close;
    win.event_paint       = handle_window_paint;
    win.event_resize      = handle_window_resize;
    win.event_gain_focus  = handle_window_gain_focus;
    win.event_lost_focus  = handle_window_lost_focus;

    win.create(wdk::window_params(cmd.surface_width, cmd.surface_height, "Events", 0, cmd.fullscreen, style));

    // keyboard access
    wdk::keyboard kb(disp);
    kb.event_keydown = std::bind(handle_keydown, std::placeholders::_1, std::ref(kb), std::ref(win));
    kb.event_keyup   = handle_keyup;

    // character input
    wdk::ime im(disp, cmd.ime_output);
    im.event_char = std::bind(handle_character, std::placeholders::_1, std::ref(im));

    // event loop
    while  (win.exists())
    {
        dispatch_one(disp, win, kb, im);
    }
    return 0;
}


