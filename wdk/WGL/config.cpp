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
#include <vector>
#include <cassert>
#include "../config.h"
#include "../context.h"
#include "../utility.h"
#include "../win32/helpers.h"

// http://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt
// Accepted in the <piAttributes> parameter array of
// wglGetPixelFormatAttribivARB, and wglGetPixelFormatAttribfvARB, and
// as a type in the <piAttribIList> and <pfAttribFList> parameter
// arrays of wglChoosePixelFormatARB :
#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NEED_PALETTE_ARB                    0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
#define WGL_SWAP_METHOD_ARB                     0x2007
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_TRANSPARENT_ARB                     0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
#define WGL_SHARE_DEPTH_ARB                     0x200C
#define WGL_SHARE_STENCIL_ARB                   0x200D
#define WGL_SHARE_ACCUM_ARB                     0x200E
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_AUX_BUFFERS_ARB                     0x2024

// Accepted as a value in the <piAttribIList> and <pfAttribFList>
// parameter arrays of wglChoosePixelFormatARB, and returned in the
// <piValues> parameter array of wglGetPixelFormatAttribivARB, and the
// <pfValues> parameter array of wglGetPixelFormatAttribfvARB :
#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

// http://www.opengl.org/registry/specs/ARB/wgl_pbuffer.txt
// Accepted by the <attribute> parameter of wglChoosePixelFormatEXT:
#define WGL_DRAW_TO_PBUFFER_ARB                 0x202D

namespace {
    void set_if(std::vector<wdk::uint_t>& v, wdk::uint_t attr, wdk::uint_t value)
    {
        if (value)
        {
            v.push_back(attr);
            v.push_back(value);
        }
    }
    typedef BOOL (APIENTRY *wglChoosePixelFormatARBProc)(HDC hdc, const int* piAttribIList,  const FLOAT* pfAttribFList, UINT nMaxFormats,  int* piFormats, UINT* nNumFormats);

} // namespace

namespace wdk
{


config::attributes config::DONT_CARE = {0, 0, 0, 0, 0, 0, 0, false, {true, false, false}};
config::attributes config::DEFAULT = {8, 8, 8, 8, 16, 0, 0, true, {true, false, false}};

struct config::impl {
    int pixelformat;
    PIXELFORMATDESCRIPTOR desc;
};

config::config(const attributes& attrs) : pimpl_(new impl)
{
    auto wglChoosePixelFormat = reinterpret_cast<wglChoosePixelFormatARBProc>(context::resolve("wglChoosePixelFormatARB"));
    if (!wglChoosePixelFormat)
        throw std::runtime_error("unable to choose framebuffer format. no wglChoosePixelFormatARB");

    dummywin win;

    int pixelformat = attrs.visualid ? attrs.visualid : 0;
    if (!pixelformat)
    {
        std::vector<uint_t> criteria = 
        {
            WGL_SUPPORT_OPENGL_ARB, TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB
        };

        set_if(criteria, WGL_RED_BITS_ARB, attrs.red_size);
        set_if(criteria, WGL_GREEN_BITS_ARB, attrs.green_size);
        set_if(criteria, WGL_BLUE_BITS_ARB, attrs.blue_size);
        set_if(criteria, WGL_ALPHA_BITS_ARB, attrs.alpha_size);
        set_if(criteria, WGL_DEPTH_BITS_ARB, attrs.depth_size);

        assert(!attrs.surfaces.pbuffer && "pbuffer is not implemented");

        if (attrs.double_buffer)
            set_if(criteria, WGL_DOUBLE_BUFFER_ARB, TRUE);

        set_if(criteria, WGL_DRAW_TO_WINDOW_ARB, (uint_t)attrs.surfaces.window);
        set_if(criteria, WGL_DRAW_TO_BITMAP_ARB, (uint_t)attrs.surfaces.pixmap);
        set_if(criteria, WGL_DRAW_TO_PBUFFER_ARB, (uint_t)attrs.surfaces.pbuffer);

		const int ARNOLD = 0;

		criteria.push_back(ARNOLD);

        UINT num_matches = 0;
        if (!wglChoosePixelFormat(win.surface(), (const int*)&criteria[0], nullptr, 1, &pixelformat, &num_matches) || !num_matches)
            throw std::runtime_error("no matching framebuffer configuration available");
    }

    pimpl_->pixelformat = pixelformat;

    const int ret = DescribePixelFormat(win.surface(), pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pimpl_->desc);

    assert(ret);
}

config::~config()
{
}

uint_t config::visualid() const
{
    return pimpl_->pixelformat;
}

uint_t config::configid() const
{
    return pimpl_->pixelformat;
}


gl_config_t config::handle() const
{
    return gl_config_t { &pimpl_->desc };
}

} // wdk