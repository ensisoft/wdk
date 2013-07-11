

#pragma once

#include <memory>
#include <vector>
#include <iostream>
#include "types.h"
#include "utility.h"

namespace wdk
{
    struct videomode {
        uint_t         xres;
        uint_t         yres;
        native_vmode_t id;
    };

    inline
    std::ostream& operator<<(std::ostream& o, const videomode& vm)
    {
        o << "VideoMode: (" << vm.id << ") " << vm.xres << "x" << vm.yres;
        return o;
    }

    struct event;

    // create connection to a display server.
    class display : noncopyable
    {
    public:
        display();
       ~display();
        
        // get current videomode setting
        videomode get_current_video_mode() const;
        
        // change the current display server video mode to the mode
        // identified by the given id. throws an exception if this fails.
        void set_video_mode(native_vmode_t id);

        // get a list of available video modes.
        void list_video_modes(std::vector<videomode>& modes);

        // check if there is a pending event for any of the windows
        // created with this display object
        bool has_event() const;
        
        // get the next event from the queue. this call will block
        // the caller untill there's an event to retrieve from the queue.
        void get_event(event& ev);
        
        // get the next event from the queue if any. this call will
        // return immediately and returns true if event was retrieved
        // otherwise it returns false.
        bool peek_event(event& ev) const;

        // block the caller untill an event is available.
        // returns true if new event arrived, otherwise false on timeout
        bool wait_for_events(ms_t timeout = wdk::NO_TIMEOUT) const;

        // block the caller untill an event is available
        // or any of the specified handles are signaled.
        // returns true if new event arrived, otherwise false on timeout
        bool wait_for_events(native_handle_t* handles, uint_t num_handles, native_handle_t** signaled, ms_t timeout = wdk::NO_TIMEOUT) const;
        
        // get current display width in pixels
        uint_t width() const;
        
        // get current display height in pixels
        uint_t height() const;
        
        // get native display handle
        native_display_t handle() const;

    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

    void dispose(const event& ev);
} // wdk
