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
#include <stdexcept>
#include <cassert>

#include "wdk/pixmap.h"
#include "wdk/system.h"
#include "wdk/utility.h"

namespace wdk
{
struct Pixmap::impl {
    HBITMAP bmp;
    uint_t  width;
    uint_t  height;
    uint_t  depth;
};

Pixmap::Pixmap(uint_t width, uint_t height, uint_t visualid)
{
    assert(width && height);
    assert(visualid);

    HDC dpy = GetNativeDisplayHandle();

    auto hdc = MakeUniqueHandle(CreateCompatibleDC(dpy), DeleteDC);
    if (!hdc.get())
        throw std::runtime_error("create compatible dc failed");

    PIXELFORMATDESCRIPTOR desc = {0};
    DescribePixelFormat(hdc.get(), visualid, sizeof(desc), &desc);
    if (!SetPixelFormat(hdc.get(), visualid, &desc))
        throw std::runtime_error("set pixelformat failed");

    auto bmp = MakeUniqueHandle(CreateCompatibleBitmap(hdc.get(), width, height), DeleteObject);
    if (!bmp.get())
        throw std::runtime_error("create bitmap failed");

    pimpl_.reset(new impl);
    pimpl_->bmp    = bmp.release();
    pimpl_->width  = width;
    pimpl_->height = height;
    pimpl_->depth  = desc.cColorBits;
}

Pixmap::~Pixmap()
{
    DeleteObject(pimpl_->bmp);
}

native_pixmap_t Pixmap::GetNativeHandle() const
{
    return pimpl_->bmp;
}

uint_t Pixmap::GetWidth() const
{
    return pimpl_->width;
}

uint_t Pixmap::GetHeight() const
{
    return pimpl_->height;
}

uint_t Pixmap::GetBitDepth() const
{
    return pimpl_->depth * 8;
}

} // wdk
