  
#pragma once

#include <functional>   // for function, bind
#include <memory>       // for unique_ptr
#include <string>       
#include "types.h"
#include "utility.h"

namespace wdk
{
    enum window_properties {
        WP_BORDER   = 0x1,
        WP_RESIZE   = 0x2
    };

    // window parameters defines how the window is to be created.
    // if fullscreen is true then window properties are ignored
    // and window will be created without border, unresizeable and 
    // dimensions will be current video mode dimensions.
    struct window_param {
        std::string title;        // optional window title to be display in title bar
        uint_t      width;        // inside width of the window, i.e. the surface width
        uint_t      height;       // inside height of the window, i.e. the surface height
        uint_t      visualid;     // visualid to be used, 0 for don't care
        bitflag_t   props;        // window properties
        bool        fullscreen;   // fullscreen flag
    };
 
    struct event;   

    // window events
    struct window_event_create;
    struct window_event_paint;
    struct window_event_resize;
    struct window_event_focus;
    struct window_event_close;
    struct window_event_destroy;

    // create a window for showing/rendering content 
    class window : noncopyable
    {
    public:
        std::function<void (const window_event_create&)>  event_create;
        std::function<void (const window_event_paint&)>   event_paint;
        std::function<void (const window_event_resize&)>  event_resize;
        std::function<void (const window_event_focus&)>   event_lost_focus;
        std::function<void (const window_event_focus&)>   event_gain_focus;
        std::function<void (const window_event_destroy&)> event_destroy;
        std::function<void (window_event_close&)>         event_close;

        window(native_display_t disp);
       ~window();
        
        // create the window.
        void create(const window_param& how);

        // close the window. 
        void close();
        
        // get current window width in screen units. 
        uint_t width() const;

        // get current window height in screen  units
        uint_t height() const;
        
        // get the current window visual id. 
        uint_t visualid() const;

        // get drawable surface width in px
        uint_t surface_width() const;
        
        // get drawable surface height in px
        uint_t surface_height() const;

        // returns true if window exists otherwise false
        bool exists() const;

        // dispatch the given event. returns true
        // if message was dispatched, otherwise false
        bool dispatch_event(const event& ev);

        // get native window handle
        native_window_t handle() const;
        
        // get native drawable/surface handle
        native_surface_t surface() const;

        // get the display handle where the window is created
        native_display_t display() const;
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

    // interface for listening to window events
    class window_listener
    {
    public:
        virtual ~window_listener() {}
        virtual void on_create(const window_event_create&) {}
        virtual void on_paint(const window_event_paint&) {}
        virtual void on_resize(const window_event_resize&) {}
        virtual void on_lost_focus(const window_event_focus&) {}
        virtual void on_gain_focus(const window_event_focus&) {}
        virtual void on_close(window_event_close&) {}
        virtual void on_destroy(const window_event_destroy&) {}

    };

    // connect all events in the given window to the given window listener
    inline void connect(window& win, window_listener* listener)
    {
        namespace args = std::placeholders;
        win.event_create = std::bind(&window_listener::on_create, listener, args::_1);
        win.event_paint  = std::bind(&window_listener::on_paint, listener, args::_1);
        win.event_resize = std::bind(&window_listener::on_resize, listener, args::_1);
        win.event_lost_focus = std::bind(&window_listener::on_lost_focus, listener, args::_1);
        win.event_gain_focus = std::bind(&window_listener::on_gain_focus, listener, args::_1);
        win.event_close = std::bind(&window_listener::on_close, listener, args::_1);
        win.event_destroy = std::bind(&window_listener::on_destroy, listener, args::_1);
    }

} // wdk
