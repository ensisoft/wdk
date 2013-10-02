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

#pragma once

#include <vector>
#include <string>
#include "types.h"

namespace wdk
{
    struct videomode;

    enum class keymod;
    enum class keysym;

    // get native display handle
    native_display_t get_display_handle();

     // get current videomode setting
    videomode get_current_video_mode();

    // change the current display server video mode to the mode
    // identified by the given id. throws an exception if this fails.
    // videomode should be one of the modes available in the list of modes
    void set_video_mode(const videomode& m);

    // get a list of available video modes.
    std::vector<videomode> list_video_modes();

    // check if there are any pending events
    bool have_events();

    bool sync_events();

    // get the next event from the queue if any. this call will
    // return immediately and returns true if event was retrieved
    // otherwise it returns false.
    bool peek_event(native_event_t& ev);
        
    // get the next event from the queue. this call will block
    // the caller untill there's an event to retrieve from the queue.
    native_event_t get_event();

    // translate keydown event
    std::pair<keymod, keysym> translate_keydown(const native_event_t& key);
    
    bool test_key_down(keysym symbol);

    bool test_key_down(uint_t keycode);

    // get native platform depedant keycode for a key
    uint_t keysym_to_keycode(keysym symbol);

} // wdk
