// Copyright (c) 2013-2016 Sami Väisänen, Ensisoft 
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

// for clang in SublimeText2
#ifndef WDK_WAYLAND
#  define WDK_WAYLAND
#endif

#include "../pixmap.h"

namespace wdk
{

struct pixmap::impl {

};

pixmap::pixmap(uint_t width, uint_t height, uint_t visualid)
{}

pixmap::~pixmap()
{}

native_pixmap_t pixmap::handle() const 
{
    return {};
}

uint_t pixmap::width() const 
{
    return 0;
}

uint_t pixmap::height() const 
{
    return 0;
}

uint_t pixmap::depth() const 
{
    return 0;
}

} // wdk