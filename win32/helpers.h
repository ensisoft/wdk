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

#include <windows.h>
#include <cassert>
#include <memory>
#include "../types.h"
#include "../utility.h"

namespace wdk
{
    uint_t get_desktop_width();
    uint_t get_desktop_height();

    // dummy window for scenarios when a hwnd/hdc is needed but there's no "real" window yet
    class dummywin : noncopyable
    {
    public:
        dummywin() : hwnd_(nullptr), hdc_(nullptr)
        {
            hwnd_ = CreateWindow(TEXT("STATIC"), TEXT(""), WS_POPUP | WS_DISABLED, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
            hdc_  = GetDC(hwnd_);
#ifndef _NDEBUG
            RECT rc;
            GetClientRect(hwnd_, &rc);
            assert(rc.bottom == 1);
            assert(rc.right  == 1);
#endif
        }
       ~dummywin()
        {
            if (hwnd_)
            {
                ReleaseDC(hwnd_, hdc_);
                DestroyWindow(hwnd_);
            }
        }
        HWND handle() const
        {
            return hwnd_;
        }
        HDC surface() const
        {
            return hdc_;
        }
    private:
        HWND hwnd_;
        HDC  hdc_;
    };

} // wdk