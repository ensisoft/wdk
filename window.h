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

#include <functional>   // for function, bind
#include <memory>       // for unique_ptr
#include <string>       
#include "types.h"
#include "utility.h"
#include "fwddecl.h"

namespace wdk
{
    enum class window_style {
        none   = 0,
        border = 0x1,
        resize = 0x2, 
        defaults = border | resize
    };

    inline window_style operator | (window_style x, window_style y)
    {
        return static_cast<window_style>(bitflag_t(x) | bitflag_t(y));
    }
    inline window_style operator & (window_style x, window_style y)
    {
        return static_cast<window_style>(bitflag_t(x) & bitflag_t(y));
    }
    inline void operator |= (window_style& x, window_style y)
    {
        x = x | y;
    }


    // window parameters defines how the window is to be created.
    // if fullscreen is true then window properties are ignored
    // and window will be created without border, unresizeable and 
    // dimensions will be same as current display video mode resolution.
    struct window_params {
        uint_t       width;
        uint_t       height;
        uint_t       visualid;
        window_style style;
        std::string  title;        
        bool         fullscreen;

        window_params(uint_t w, uint_t h, const std::string& str = std::string(), uint_t visual = 0, bool fs = false, window_style style_bits = window_style::defaults) : 
           width(w), height(h), visualid(visual), style(style_bits), title(str), fullscreen(fs)
        {
        }
    };

    // create a window for showing/rendering content 
    class window : noncopyable
    {
    public:
        std::function<void (const window_event_create&)>   event_create;
        std::function<void (const window_event_paint&)>    event_paint;
        std::function<void (const window_event_resize&)>   event_resize;
        std::function<void (const window_event_focus&)>    event_lost_focus;
        std::function<void (const window_event_focus&)>    event_gain_focus;
        std::function<void (window_event_query_close&)>    event_query_close;
        std::function<void (const window_event_destroy&)>  event_destroy;

        window(const display& disp);
       ~window();

        // create the window.
        void create(const window_params& how);

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
        bool dispatch(const event& ev) const;

        // get native window handle
        native_window_t handle() const;

        // get the display handle where the window is created
        native_display_t display() const;
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

} // wdk
