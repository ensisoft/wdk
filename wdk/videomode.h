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

#include <iostream>
#include "types.h"

namespace wdk
{
    struct videomode {
        uint_t xres;
        uint_t yres;
        videomode() : xres(0), yres(0)
        {}
        videomode(uint_t width, uint_t height) : xres(width), yres(height)
        {}        
        bool is_empty() const
        {
            return xres == 0 && yres == 0;
        }
    };

    inline bool operator < (const videomode& lhs, const videomode& rhs)
    {
        return (lhs.xres * lhs.yres) < (rhs.xres * rhs.yres);
    }

    inline
    bool operator > (const videomode& lhs, const videomode& rhs)
    {
        return (rhs < lhs);
    }

    inline
    bool operator == (const videomode& lhs, const videomode& rhs)
    {
        return !(lhs < rhs) && !(rhs < lhs);
    }

    inline
    bool operator != (const videomode& lhs, const videomode& rhs)
    {
        return !(lhs == rhs);
    }

    inline
    std::ostream& operator << (std::ostream& o, const videomode& vm)
    {
        o << "VideoMode: " << vm.xres << "x" << vm.yres;
        return o;
    }

} // wdk