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
    typedef MSG     native_event_t;
    typedef HDC     native_display_t;
    typedef HDC     native_surface_t;
    typedef size_t  native_vmode_t;


    const HWND    NULL_WINDOW  = NULL;
    const HANDLE  NULL_HANDLE  = NULL;
    const HDC     NULL_SURFACE = NULL;
    const HBITMAP NULL_PIXMAP  = NULL;
    const size_t DEFAULT_VIDEO_MODE = -1;

} // wdk


