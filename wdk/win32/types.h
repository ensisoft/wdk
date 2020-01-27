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

#include <windows.h>

namespace wdk
{
    typedef HWND    native_window_t;
    typedef HANDLE  native_handle_t;
    typedef HBITMAP native_pixmap_t;
    typedef HDC     native_display_t;

    class native_event_t
    {
    public:
        enum class type {
            window_gain_focus,
            window_lost_focus,
            window_resize,
            window_create,
            window_destroy,
            window_close,
            window_paint,
            window_keyup,
            window_keydown,
            window_char,
            window_mouse_move,
            window_mouse_press,
            window_mouse_release,
            system_resolution_change,
            other
        };

        native_event_t()
        {
            msg_ = MSG{0};
        }
        native_event_t(const MSG& m) : msg_(m)
        {}

        operator const MSG& () const
        {
            return msg_;
        }
        native_window_t get_window_handle() const
        {
            return msg_.hwnd;
        }
     	const MSG& get() const
        {
            return msg_;
        }

        type identity() const
        {
            switch (msg_.message)
            {
                case WM_SETFOCUS:      return type::window_gain_focus;
                case WM_KILLFOCUS:     return type::window_lost_focus;
                case WM_PAINT:         return type::window_paint;
                case WM_SIZE:          return type::window_resize;
                case WM_CREATE:        return type::window_create;
                case WM_DESTROY:       return type::window_destroy;
                case WM_CLOSE:         return type::window_close;
                case WM_KEYDOWN:       return type::window_keydown;
                case WM_KEYUP:         return type::window_keyup;
                case WM_CHAR:          return type::window_char;
                case WM_DISPLAYCHANGE: return type::system_resolution_change;
                case WM_MOUSEMOVE:     return type::window_mouse_move;

                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                    return type::window_mouse_press;

                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                    return type::window_mouse_release;

                default:
                    break;
            }
            return type::other;
        }
    private:
        MSG msg_;
    };

} // wdk


