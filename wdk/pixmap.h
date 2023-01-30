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

#include <memory>

#include "wdk/types.h"

namespace wdk
{
    // Window system provided bitmap
    class Pixmap
    {
    public:
        // Create a new bitmap with the given width, height
        // and visual id. 
        Pixmap(uint_t width, uint_t height, uint_t visualid);
       ~Pixmap();
        // Get the pixmap's platform specific native handle.
        // On X11 this is Pixmap, on Win32 this is HBITMAP
        native_pixmap_t GetNativeHandle() const;

        // Get bitmap width.
        uint_t GetWidth() const;
        // Get bitmap height.
        uint_t GetHeight() const;
        // Get bitmap depth in bits.
        uint_t GetBitDepth() const;
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
   };

} // wdk
