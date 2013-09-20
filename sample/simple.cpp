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

#ifdef SAMPLE_UTF8_TRANSLATE
#  include <enc/utf8.h>
#endif
#include <wdk/display.h>
#include <wdk/window.h>
#include <wdk/events.h>
#include <wdk/keyboard.h>
#include <wdk/event.h>
#include <iostream>
#include <cassert>
#include <cstring>
#ifdef LINUX
#  include <X11/Xlib.h> // for native drawing
#endif

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

void handle_char(const wdk::keyboard_event_char& uchar)
{
    printf("Char event: U+%x", (int)uchar.value);

#ifdef SAMPLE_UTF8_TRANSLATE
    std::string utf8;
    enc::utf8_encode(&uchar.value, &uchar.value + 1, std::back_inserter(utf8));
    printf(" => \"%s\"", utf8.c_str());
#endif

    printf("\n");
}

void handle_window_create(const wdk::window_event_create& create)
{
    printf("Window create: @%d,%d, %d x %d\n", create.x, create.y, create.width, create.height);
}

void handle_window_query_close(wdk::window_event_query_close& query_close)
{
    printf("Window close:\n");
    query_close.should_close = true;
}

void handle_window_paint(const wdk::window_event_paint& paint)
{
    printf("Window paint: @%d,%d, %dx%d\n", paint.x, paint.y, paint.width, paint.height);

#ifdef WINDOWS
    HDC hdc = GetDC(paint.window);    

    RECT rc = {0};
    rc.top = paint.y;
    rc.left = paint.x;
    rc.right = paint.x + paint.width;
    rc.bottom = paint.y + paint.height;

    HBRUSH green_brush = CreateSolidBrush(RGB(0, 0x80, 0));

    FillRect(hdc, &rc, green_brush);

    DeleteObject(green_brush);
    ReleaseDC(paint.window, hdc);

#elif defined(LINUX)
    Colormap colormap = DefaultColormap(paint.display, 0);

    GC green_gc = XCreateGC(paint.display, paint.window, 0, NULL);

    XColor green;
    XParseColor(paint.display, colormap, "#008000", &green);
    XAllocColor(paint.display, colormap, &green);

    XSetForeground(paint.display, green_gc, green.pixel);
    XFillRectangle(paint.display, paint.window, green_gc, paint.x, paint.y, paint.width, paint.height);

    XFreeGC(paint.display, green_gc);
#endif

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

struct cmdline {
    bool   print_help;
    bool   fullscreen;
    bool   listmodes;
    bool   wnd_border;
    bool   wnd_resize;
    bool   wnd_move;
    int    surface_width;
    int    surface_height;
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
        else if (!strcmp(name, "--wnd-no-move"))
            cmd.wnd_move = false;
        else
        {
            if (!(i + 1 < argc))
                return false;

            long value = atoi(argv[++i]);
            if (!strcmp(name, "--wnd-width"))
                cmd.surface_width = value;
            else if (!strcmp(name, "--wnd-height"))
                cmd.surface_height = value;
            else if (!strcmp(name, "--video-mode"))
                cmd.videomode = static_cast<wdk::native_vmode_t>(value);
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
    cmd.wnd_move       = true;
    cmd.surface_width  = 640;
    cmd.surface_height = 480;
    cmd.videomode      = wdk::DEFAULT_VIDEO_MODE;

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

    // window
    wdk::window win(disp);

    wdk::window::params param;
    param.title      = "Simple Window";
    param.width      = cmd.surface_width;
    param.height     = cmd.surface_height;
    param.visualid   = 0;
    param.fullscreen = cmd.fullscreen;
    param.props      = 0;
    if (cmd.wnd_border)
        param.props |= wdk::window::HAS_BORDER;
    if (cmd.wnd_resize)
        param.props |= wdk::window::CAN_RESIZE;
    if (cmd.wnd_move)
        param.props |= wdk::window::CAN_MOVE;

    win.event_create      = handle_window_create;
    win.event_query_close = handle_window_query_close;
    win.event_paint       = handle_window_paint;
    win.event_resize      = handle_window_resize;
    win.event_gain_focus  = handle_window_gain_focus;
    win.event_lost_focus  = handle_window_lost_focus;

    win.create(param);

    // keyboard access
    wdk::keyboard kb(disp);

    kb.event_keydown = std::bind(handle_keydown, std::placeholders::_1, std::ref(kb), std::ref(win));
    kb.event_keyup   = handle_keyup;
    kb.event_char    = handle_char;

    // event loop
    while  (win.exists())
    {
        wdk::event e {0};
        disp.get_event(e);

        if (!win.dispatch_event(e))
            kb.dispatch_event(e);

        dispose(e);
    }
    return 0;
}


