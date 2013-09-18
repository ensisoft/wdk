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
#include <GL/gl.h>
#include <cassert>
#include <stdexcept>
#include <functional>
#include <vector>
#include "../context.h"
#include "../utility.h"

// http://www.opengl.org/registry/specs/ARB/wgl_create_context.txt
// Accepted as an attribute name in <*attribList>:
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

// Accepted as bits in the attribute value for WGL_CONTEXT_FLAGS in
// <*attribList>:
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

// Accepted as bits in the attribute value for
// WGL_CONTEXT_PROFILE_MASK_ARB in <*attribList>:
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// New errors returned by GetLastError :
#define ERROR_INVALID_VERSION_ARB               0x2095
#define ERROR_INVALID_PROFILE_ARB               0x2096


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


namespace {
    class dummy_window 
    {
    public:
        dummy_window() : hwnd_(NULL), hdc_(NULL)
        {
        }
        dummy_window(dummy_window&& other) : hwnd_(other.hwnd_), hdc_(other.hdc_)
        {
            other.hwnd_ = NULL;
            other.hdc_  = NULL;
        }
        void create()
        {
            WNDCLASSEX cls = { 0 };
            cls.cbSize        = sizeof(cls);
            cls.lpfnWndProc   = DefWindowProc;
            cls.lpszClassName = TEXT("context-dummy-window");
            RegisterClassEx(&cls);

            hwnd_ = CreateWindowEx(
                WS_EX_APPWINDOW,
                TEXT("context-dummy-window"),
                NULL,
                WS_POPUP,
                0, 0,
                1, 1,
                NULL,
                NULL,
                NULL,
                NULL);
            if (!hwnd_)
                throw std::runtime_error("create window failed");

            hdc_ = GetDC(hwnd_);
        }

       ~dummy_window()
        {
            if (!hwnd_)
                return;
            
            BOOL ret;

            ret = ReleaseDC(hwnd_, hdc_);
            assert(ret);

            ret = DestroyWindow(hwnd_);
            assert(ret);
        }
        HWND window() 
        {
            return hwnd_;
        }
        HDC surface() 
        {
            return hdc_;
        }
        dummy_window& operator=(dummy_window&& other)
        {
            dummy_window t(std::move(*this));
            std::swap(hwnd_, other.hwnd_);
            std::swap(hdc_, other.hdc_);
            return *this;
        }
    private:

        HWND hwnd_;
        HDC  hdc_;
    };
} // namespace

namespace wdk
{

context::attributes::attributes() : 
    major_version(3),
    minor_version(1),
    red_size(8),
    green_size(8),
    blue_size(8),
    alpha_size(8),
    depth_size(16),
    render_window(true),
    render_pixmap(false),
    doublebuffer(true),
    visualid(0)
{
}


struct context::impl 
{
    HGLRC   context;
    HDC     surface;    
    dummy_window temp_window;   
    int     pixelformat;

    void release_surface() 
    {
        if (!surface)
            return;
        
        HWND hwnd = WindowFromDC(surface);
        BOOL ret = ReleaseDC(hwnd, surface);
        assert(ret);

        surface = NULL;
    }
};

context::context(native_display_t disp, const attributes& attrs)
{
    assert(!attrs.render_pixmap && "render to pixmap is not supported on win32");

    typedef HGLRC (APIENTRY *wglCreateContextAttribsARBProc)(HDC, HGLRC, const int*);

    typedef BOOL (APIENTRY *wglChoosePixelFormatARBProc)(HDC hdc, 
        const int* piAttribIList, 
        const FLOAT* pfAttribFList, 
        UINT nMaxFormats, 
        int* piFormats, 
        UINT* nNumFormats);

    // resolve extensions needed
    auto wglCreateContextAttribsARB = reinterpret_cast<wglCreateContextAttribsARBProc>(resolve("wglCreateContextAttribsARB"));
    auto wglChoosePixelFormatARB = reinterpret_cast<wglChoosePixelFormatARBProc>(resolve("wglChoosePixelFormatARB"));

    if (!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
        throw std::runtime_error("missing WGL extensions for GL context creation");
    
    // create temporary/dummy window/bitmap for the context creation.
    // we use the dummy window to set the required pixelformat 
    // of the context configuration. subsequently once the context 
    // has a pixelformat any call to make_current expects that the 
    // rendering surface (window, pixmap) has the same pixelformat, 
    // otherwise the call will fail.
    dummy_window window;
    window.create();
    
    HDC surface = window.surface();

    int sel_pixelformat = attrs.visualid ? attrs.visualid : 0;  

    const uint_t ARNOLD = 0; // attrib list terminator

    if (!sel_pixelformat)
    {
        const uint_t color_bits = attrs.red_size + attrs.green_size + attrs.blue_size + attrs.alpha_size;
    
        // attribute list for color buffer configuration
        const uint_t window_config[] = {
            WGL_SUPPORT_OPENGL_ARB, TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, color_bits,
            WGL_RED_BITS_ARB, attrs.red_size,
            WGL_GREEN_BITS_ARB, attrs.green_size,
            WGL_BLUE_BITS_ARB, attrs.blue_size,
            WGL_ALPHA_BITS_ARB, attrs.alpha_size,
            WGL_DEPTH_BITS_ARB, attrs.depth_size,
            WGL_DOUBLE_BUFFER_ARB, (uint_t)attrs.doublebuffer,          
            WGL_DRAW_TO_WINDOW_ARB, TRUE,
            ARNOLD
        };      

        int num_pixelformat = 0;        
        if (!wglChoosePixelFormatARB(surface, (const int*)&window_config, NULL, 1, &sel_pixelformat, (UINT*)&num_pixelformat))
            throw std::runtime_error("no such GL color buffer configuration");

        assert(sel_pixelformat);
    }

    assert(surface);
    assert(sel_pixelformat);

    PIXELFORMATDESCRIPTOR desc = { 0 };
    const int ret = DescribePixelFormat(surface, sel_pixelformat, sizeof(desc), &desc);
        
    if (!SetPixelFormat(surface, sel_pixelformat, &desc))
        throw std::runtime_error("set pixelformat failed");

    const uint_t context_config[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, attrs.major_version,
        WGL_CONTEXT_MINOR_VERSION_ARB, attrs.minor_version,
        ARNOLD
    };

    // now create the context and make it current
    auto context = make_unique_ptr(wglCreateContextAttribsARB(surface, NULL, (const int*)&context_config), wglDeleteContext);
    if (!context)
        throw std::runtime_error("create context failed");

    if (!wglMakeCurrent(surface, context.get()))
        throw std::runtime_error("make current failed");

    pimpl_.reset(new impl);
    pimpl_->context           = context.release();
    pimpl_->surface           = NULL;
    pimpl_->temp_window       = std::move(window);
    pimpl_->pixelformat       = sel_pixelformat;
}

context::~context()
{
    wglMakeCurrent(NULL, NULL);

    pimpl_->release_surface();

    wglDeleteContext(pimpl_->context);

}

void context::make_current(native_window_t window)
{
    // wgl has a problem similar to glX that you can't pass NULL for HDC.
    // so we use the temporary window surface
    wglMakeCurrent(pimpl_->temp_window.surface(), pimpl_->context);

    pimpl_->release_surface();

    if (window == wdk::NULL_WINDOW)
        return;

    HDC hdc = GetDC(window);

    if (!wglMakeCurrent(hdc, pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = hdc;    
}

void context::make_current(native_pixmap_t pixmap)
{
    assert(!"not supported");
}

void context::swap_buffers()
{
    assert(pimpl_->surface && "context has no valid surface. did you forget to call make_current?");
    
    const BOOL ret = SwapBuffers(pimpl_->surface);

    assert(ret == TRUE);
}

uint_t context::visualid() const
{
    return pimpl_->pixelformat;
}

bool context::has_dri() const
{
    return true;
}

void* context::resolve(const char* function)
{
    assert(function && "null function name");
    
    if (!wglGetCurrentContext())
    {
        // wglGetProcAddress won't work unless there's a current context
        // so we have a special dummy context created with a dummy window
        // and made current just so that we can query for functions
        // with wglGetProcAddress.  (nice huh??)
        // note that creating a compatible (empty) DC with CreateCompatibleDC
        // wont work but will crash in makeCurrent (nvidia)
        struct dummy_context 
        {
            dummy_context() : hgl(NULL)
            {
                wnd.create();

                PIXELFORMATDESCRIPTOR desc = {0};
                desc.nVersion   = 1;
                desc.dwFlags    = PFD_SUPPORT_OPENGL;

                const int pixelformat = ChoosePixelFormat(wnd.surface(), &desc);
                if (!SetPixelFormat(wnd.surface(), pixelformat, &desc))
                    throw std::runtime_error("horrors");

                hgl = wglCreateContext(wnd.surface());
                if (!hgl)
                    throw std::runtime_error("horrors again");
            }
           ~dummy_context()
            {              
                BOOL ret = wglDeleteContext(hgl);
                assert(ret);
            }
            void make_current()
            {
                BOOL ret = wglMakeCurrent(wnd.surface(), hgl);
                assert(ret);
            }
        private:
            dummy_window wnd;
            HGLRC hgl;
        };

        static dummy_context dummy;

        dummy.make_current();
    }
    
    void* ret = (void*)wglGetProcAddress(function);
    
    return ret;
}

} // wdk
