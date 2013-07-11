

#ifdef SAMPLE_USE_BOOST
#  include <boost/program_options.hpp>
#endif
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
#ifdef LINUX
#  include <X11/Xlib.h> // for native drawing
#endif

#ifdef SAMPLE_USE_BOOST
  namespace po = boost::program_options;
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

void handle_window_close(wdk::window_event_close& query_close)
{
    printf("Window close:\n");
    query_close.should_close = true;
}

void handle_window_paint(const wdk::window_event_paint& paint)
{
//    printf("Window paint: @%d,%d, %dx%d\n", paint.x, paint.y, paint.width, paint.height);

#ifdef WINDOWS
        RECT rc = {0};
        rc.top = paint.y;
        rc.left = paint.x;
        rc.right = paint.x + paint.width;
        rc.bottom = paint.y + paint.height;
        HBRUSH green_brush = CreateSolidBrush(RGB(0, 0x80, 0));
        FillRect(paint.surface, &rc, green_brush);
        DeleteObject(green_brush);
#elif defined(LINUX)
        Colormap colormap = DefaultColormap(paint.display, 0);
        GC green_gc = XCreateGC(paint.display, paint.surface, 0, NULL);
        XColor green;
        XParseColor(paint.display, colormap, "#008000", &green);
        XAllocColor(paint.display, colormap, &green);
        XSetForeground(paint.display, green_gc, green.pixel);
        XFillRectangle(paint.display, paint.surface, green_gc, paint.x, paint.y, paint.width, paint.height);
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


int main(int argc, char* argv[])
{
    try 
    {
        wdk::window_param param;
        param.title      = "Simple Window";
        param.width      = 640;
        param.height     = 480;
        param.visualid   = 0;
        param.props      = 0;
        param.fullscreen = false;

        wdk::uint_t mode = wdk::DEFAULT_VIDEO_MODE;
        bool border      = true;
        bool resize      = true;

        // connect to display server
        wdk::display disp;

#ifdef SAMPLE_USE_BOOST
        po::options_description desc("Options");

        desc.add_options()
        ("width", po::value<wdk::uint_t>(&param.width)->default_value(640), "Window width")
        ("height", po::value<wdk::uint_t>(&param.height)->default_value(480), "Window height")
        ("border", po::value<bool>(&border)->default_value(true), "Border")
        ("resize", po::value<bool>(&resize)->default_value(true), "Resize")
        ("mode", po::value<wdk::uint_t>(&mode)->default_value(wdk::DEFAULT_VIDEO_MODE), "Videomode")
        ("fullscreen", "Fullscreen mode")
        ("listmodes", "List video modes")
        ("help", "Print help");
        
        po::variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc;
            return 0;
        }
        else if (vm.count("listmodes"))
        {
            std::vector<wdk::videomode> modes;
            disp.list_video_modes(modes);
            std::copy(modes.begin(), modes.end(), std::ostream_iterator<wdk::videomode>(std::cout, "\n"));
            return 0;
        }

        if (vm.count("fullscreen"))
            param.fullscreen = true;
#endif
        if (border)
            param.props |= wdk::WP_BORDER;
        if (resize)
            param.props |= wdk::WP_RESIZE;

        // try to change video mode
        if (mode != wdk::DEFAULT_VIDEO_MODE)
            disp.set_video_mode(mode);

        // create our window
        wdk::window win(disp.handle());
        win.event_create  = handle_window_create;
        win.event_close   = handle_window_close;
        win.event_paint   = handle_window_paint;
        win.event_resize  = handle_window_resize;
        win.event_destroy = handle_window_destroy;
        win.event_gain_focus = handle_window_gain_focus;
        win.event_lost_focus = handle_window_lost_focus;

        win.create(param);

        // keyboard access
        wdk::keyboard kb(disp.handle());
        kb.event_keydown = std::bind(handle_keydown, std::placeholders::_1, std::ref(kb), std::ref(win));
        kb.event_keyup   = handle_keyup;
        kb.event_char    = handle_char;

        while (win.exists())
        {
            wdk::event e = {0};
            disp.get_event(e);

            // event dispatching
            if (!win.dispatch_event(e))
                kb.dispatch_event(e);

            dispose(e);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Oops, something went wrong\n"
        << e.what() << std::endl;
        return 1;
    }
    return 0;
}

