

#pragma once

namespace wdk
{
    enum class event_type {
    	none,
    	window_create,
    	window_paint,
    	window_configure,
    	window_gain_focus,
    	window_lost_focus,
    	window_close,
    	window_destroy,
    	keyboard_keyup,
    	keyboard_keydown,
    	keyboard_char,
    	mouse_move
    };

    // native window system event
    struct event {
        native_window_t window;
        native_event_t  ev;
        event_type type;
    };
	
    inline
    bool is_window_event(event_type e)
    {
        return (e >= event_type::window_create && e <= event_type::window_close);
    }

    inline
    bool is_keyboard_event(event_type e)
    {
        return (e >= event_type::keyboard_keyup && e <= event_type::keyboard_char);
    }

} // wdk