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
    enum class Keymod;
    enum class Keysym;

    // Get native display handle. 
    // On X11 based system this is the Display object
    // on Win32 this is the HDC to the desktop. 
    // You should not mess with this unless you know what you're doing.
    native_display_t GetNativeDisplayHandle();

     // Get the current videomode setting.
    VideoMode GetCurrentVideoMode();

    // Request the system to change the current display video mode. 
    // If the mode is not valid or the change is rejected by the display/driver
    // an exception is thrown.
    // Supported video modes can be listed with a call to ListVideoModes.
    void SetVideoMode(const VideoMode& m);

    // Get a list of available video modes.
    std::vector<VideoMode> ListVideoModes();

    // Get the next application event from the queue if any. 
    // returns true if event was available and assignes the
    // event into ev. Otherwise returns false and no event 
    // was immediately available. If an event was available
    // it is removed from the application's event queue.
    bool PeekEvent(native_event_t &ev);

    // Get the next application event from the event queue. 
    // Will block until an event is posted. 
    void WaitEvent(native_event_t& ev);

    // Event translation.

    // Translate system keydown event to key modifier and key symbol.
    std::pair<bitflag<Keymod>, Keysym> TranslateKeydownEvent(const native_event_t& key);

    // Translate system mouse event to mouse button and key modifier.
    std::pair<bitflag<Keymod>, MouseButton> TranslateMouseButtonEvent(const native_event_t& bnt);

    // Test if the keyboard key identified by 'symbol' is currently down.
    // Returns true if down, otherwise false.
    bool TestKeyDown(Keysym symbol);

    // Test if the keyboard key identified by the native keycode is 
    // currently down. Returns true if down, otherwise false.
    // Note that the keycodes are platform and keyboard specific. 
    // In order to use this properly the application should not assume
    // any particular keycodes but map each virtual key to the 
    // keycode using MapKeysymToNativeKeycode.
    // If an application wants to let the user rebind the keys, i.e. record
    // their own action keys, the application should first ask the user to
    // press a key for some action and then record the native key code that 
    // is in the keyboard event and then later use this key code in a
    // call to test whether the key is down or not.
    bool TestKeyDown(uint_t keycode);

    // Map virtual key symbol to a platform/keyboard specific keycode. 
    // The keycodes are *NOT* portable between systems. It'd be wise
    // for the application to stick to virtual key symbols as much possible
    // as those are designed to be portable.
    uint_t MapKeysymToNativeKeycode(Keysym symbol);

} // wdk

