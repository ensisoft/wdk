// Copyright (c) 2010-2016 Sami Väisänen, Ensisoft 
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

// window system types specific to Wayland.

struct wl_display;
struct wl_compositor;
struct wl_shell;
struct wl_surface;
struct wl_egl_window;
struct wl_shm;
struct wl_dummy;

namespace wdk
{
    struct native_display_t {
        wl_display*    display;
        wl_compositor* compositor;
        wl_shell*      shell;
        wl_shm*        shm;
    };

    typedef wl_egl_window* native_window_t;
    typedef wl_surface*    egl_handle_t;
    typedef wl_dummy*      native_event_t;
    typedef wl_dummy*      native_pixmap_t;

} // wdk

struct wl_dummy {};