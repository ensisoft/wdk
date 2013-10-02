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
        native_event_t(const MSG& m) : msg_(m), processed_(false)
        {}
        native_event_t(native_event_t&& other) : msg_(other.msg_), processed_(other.processed_)
        {
            other.msg_ = MSG {0};
            other.processed_ = true;
        }
        ~native_event_t()
        {
            if (msg_.message == WM_PAINT)
            {
                // not sure if this is the right thing to do, but on X11
                // the application also gets a single notification when an area in the
                // window needs to repainted. On win32 if application doesn't validate
                // the region (by Begin/EndPaint) the WM_PAINT is posted repeatedly
                // as long as there's an invalid rect within the window.
                // so here we enforce the assumption that after every paint request
                // the window is valid and no more paints are posted untill re-exposed.
                RECT rcPaint;
                GetUpdateRect(msg_.hwnd, &rcPaint, FALSE);
                ValidateRect(msg_.hwnd, &rcPaint);
            }

            if (processed_)
                return;
            
            if (msg_.message == WM_CLOSE)
                return;

            DefWindowProc(msg_.hwnd, msg_.message, msg_.wParam, msg_.lParam);
        }
        operator const MSG& () const
        {
            return msg_;
        }
        void done() const
        {
            processed_ = true;
        }
        native_window_t get_window_handle() const
        { 
            return msg_.hwnd;
        }
        native_event_t& operator=(native_event_t&& other)
        {
            native_event_t tmp(std::move(*this));

            std::swap(msg_, other.msg_);
            std::swap(processed_, other.processed_);

            return *this;
        }
     	const MSG& get() const
        {
	        return msg_;
        }	

        event_type get_type() const
        {
            switch (msg_.message)
            {
                case WM_SETFOCUS:      return event_type::window_gain_focus;
                case WM_KILLFOCUS:     return event_type::window_lost_focus;
                case WM_PAINT:         return event_type::window_paint;
                case WM_SIZE:          return event_type::window_resize;
                case WM_CREATE:        return event_type::window_create;
                case WM_DESTROY:       return event_type::window_destroy;
                case WM_CLOSE:         return event_type::window_close;
                case WM_KEYDOWN:       return event_type::window_keydown;
                case WM_KEYUP:         return event_type::window_keyup;
                case WM_CHAR:          return event_type::window_char;
                case WM_DISPLAYCHANGE: return event_type::display_resolution_change;
            }
            return event_type::none;
        }
    private:
        MSG msg_;
        mutable bool processed_;
    };

} // wdk


