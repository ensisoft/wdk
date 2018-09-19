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
#include "keys.h"
#include "types.h"
#include "bitflag.h"

namespace wdk
{
    struct VideoMode;

    enum class KeyModifier;
    enum class KeySymbol;

    // get native display handle
    native_display_t GetNativeDisplayHandle();

     // get current videomode setting
    VideoMode GetCurrentVideoMode();

    // change the current display server video mode to the mode
    // identified by the given id. throws an exception if this fails.
    // videomode should be one of the modes available in the list of modes
    void SetVideoMode(const VideoMode& m);

    // get a list of available video modes.
    std::vector<VideoMode> ListAvailableVideoModes();

    // Get the next event from the application's event queue if any. returns true
    // if event was available and assignes the event into ev.
    // returns false if no event was available.
    bool PeekEvent(native_event_t &ev);

	// Wait for an event in the application's event queue. 
    void WaitEvent(native_event_t& ev);


    // translate keydown event
    std::pair<bitflag<KeyModifier>, KeySymbol> translate_keydown_event(const native_event_t& key);

    std::pair<bitflag<KeyModifier>, MouseButton> translate_mouse_button_event(const native_event_t& bnt);

    bool test_key_down(KeySymbol symbol);

    bool test_key_down(uint_t keycode);

    // get native platform depedant keycode for a key
    uint_t keysym_to_keycode(KeySymbol symbol);

} // wdk

