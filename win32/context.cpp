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
#include <stdexcept>
#include <functional>
#include "../context.h"
#include "../utility.h"

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_TERMINATOR                  0

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
    bool    surface_is_window;
    HGDIOBJ original;
    HWND    temp_window;
    HDC     temp_surface;
    int     pixelformat;

    void release_surface() 
    {
        if (!surface)
            return;
        if (surface_is_window)
        {
            HWND hwnd = WindowFromDC(surface);
            ReleaseDC(hwnd, surface);
        }
        else
        {
            assert(original);
            SelectObject(surface, original);
            DeleteDC(surface);
            original = NULL;
        }
        surface = NULL;
        surface_is_window = false;
    }
};

context::context(native_display_t disp, const attributes& attrs)
{
    typedef HGLRC (*wglCreateContextAttribsARBProc)(HDC, HGLRC, const int*);

    typedef BOOL (*wglChoosePixelFormatARBProc)(
        HDC hdc, 
        const int* piAttribIList,
        const FLOAT* pfAttribFList,
        UINT nMaxFormats,
        int* piFormats,
        UINT* nNumFormats);

    // resolve extensions needed

    wglCreateContextAttribsARBProc wglCreateContextAttribsARB = reinterpret_cast<wglCreateContextAttribsARBProc>(resolve("wglCreateContextAttribsARB"));
    wglChoosePixelFormatARBProc wglChoosePixelFormatARB = reinterpret_cast<wglChoosePixelFormatARBProc>(resolve("wglChoosePixelFormatARB"));

    if (!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
        throw std::runtime_error("missing WGL extensions for GL context creation");


    WNDCLASSEX cls    = {0};
    cls.cbSize        = sizeof(cls);
    cls.hInstance     = GetModuleHandle(NULL);
    cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    cls.lpfnWndProc   = DefWindowProc;
    cls.lpszClassName = TEXT("context-temp-window");
    RegisterClassEx(&cls);

    auto window = make_unique_ptr(CreateWindowEx(
        WS_EX_APPWINDOW,
        TEXT("context-temp-window"),
        NULL,
        WS_POPUP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1, 1,
        NULL,
        NULL, 
        NULL, 
        NULL), DestroyWindow);;
    if (window.get() == NULL)
        throw std::runtime_error("create temporary context window failed");

    auto surface = make_unique_ptr(GetDC(window.get()), std::bind(ReleaseDC, window.get(), std::placeholders::_1));

    PIXELFORMATDESCRIPTOR pxd = {0};

    int pixelformat = 0;

    if (attrs.visualid)
    {
        if (!DescribePixelFormat(surface.get(), attrs.visualid, sizeof(pxd), &pxd))
            throw std::runtime_error("no such GL config available");

        pixelformat = attrs.visualid;
    }
    else
    {
        pxd.nSize      = sizeof(pxd);
        pxd.nVersion   = 1;
        pxd.dwFlags    = PFD_SUPPORT_OPENGL;
        pxd.iPixelType = PFD_TYPE_RGBA;
        pxd.cColorBits = attrs.red_size + attrs.green_size + attrs.blue_size;
        pxd.cRedBits   = attrs.red_size;
        pxd.cGreenBits = attrs.green_size;
        pxd.cBlueBits  = attrs.blue_size;
        pxd.cAlphaBits = attrs.alpha_size;
        pxd.cDepthBits = attrs.depth_size;

        if (attrs.render_window)
            pxd.dwFlags |= PFD_DRAW_TO_WINDOW;
        if (attrs.render_pixmap)
            pxd.dwFlags |= PFD_DRAW_TO_BITMAP;

        if (attrs.doublebuffer)
            pxd.dwFlags |= PFD_DOUBLEBUFFER;

        pixelformat = ChoosePixelFormat(surface.get(), &pxd);
        if (!pixelformat)
            throw std::runtime_error("no such GL config available");
    }
    if (!SetPixelFormat(surface.get(), pixelformat, &pxd))
        throw std::runtime_error("set pixel format failed");
 
    const int context_config[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, attrs.major_version,
        WGL_CONTEXT_MINOR_VERSION_ARB, attrs.minor_version,
        WGL_CONTEXT_TERMINATOR
    };

    // now create the context and make it current
    auto context = make_unique_ptr(wglCreateContextAttribsARB(surface.get(), NULL, context_config), wglDeleteContext);
    if (!context)
        throw std::runtime_error("create context failed");

    if (!wglMakeCurrent(surface.get(), context.get()))
        throw std::runtime_error("make current failed");

    pimpl_.reset(new impl);
    pimpl_->context           = context.release();
    pimpl_->surface           = NULL;
    pimpl_->surface_is_window = false;
    pimpl_->original          = NULL;
    pimpl_->temp_window       = window.release();
    pimpl_->temp_surface      = surface.release();
    pimpl_->pixelformat       = pixelformat;
}

context::~context()
{
    wglMakeCurrent(NULL, NULL);

    pimpl_->release_surface();

    wglDeleteContext(pimpl_->context);

    ReleaseDC(pimpl_->temp_window, pimpl_->temp_surface);
    DestroyWindow(pimpl_->temp_window);
}

void context::make_current(native_window_t window)
{
    // wgl has a problem similar to glX that you can't pass NULL for HDC.
    // so we use the temporary window surface
    wglMakeCurrent(pimpl_->temp_surface, pimpl_->context);

    pimpl_->release_surface();

    if (window == wdk::NULL_WINDOW)
        return;

    HDC hdc = GetDC(window);

    if (!wglMakeCurrent(hdc, pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = hdc;
    pimpl_->surface_is_window = true;
}

void context::make_current(native_pixmap_t pixmap)
{
    // wgl has a problem similar to glX that you can't pass NULL for HDC.
    // so we use the temporary window surface
    wglMakeCurrent(pimpl_->temp_surface, pimpl_->context);

    pimpl_->release_surface();

    if (pixmap == wdk::NULL_PIXMAP)
        return;

    auto hdc = make_unique_ptr(CreateCompatibleDC(pimpl_->temp_surface), DeleteDC);
    if (!hdc.get())
        throw std::runtime_error("error");

    HGDIOBJ original = SelectObject(hdc.get(), pixmap);
    if (!original)
        throw std::runtime_error("select object failed");

    if (!wglMakeCurrent(hdc.get(), pimpl_->context))
        throw std::runtime_error("make current failed");

    pimpl_->surface = hdc.release();
    pimpl_->original = original;
    pimpl_->surface_is_window = false;
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

    // wglGetProcAddress won't work unless there's a current context
    // for the calling thread. and for that we'll need a handle to a window
    if (!wglGetCurrentContext())
    {
        struct dummy_context 
        {
            dummy_context() 
            {
                hdc = GetDC(NULL);
                hgl = wglCreateContext(hdc);
            }
           ~dummy_context()
            {
                ReleaseDC(WindowFromDC(hdc), hdc);
                wglDeleteContext(hgl);
            }
            HDC hdc;
            HGLRC hgl;
        };

        static dummy_context dummy;

        wglMakeCurrent(dummy.hdc, dummy.hgl);
    }
    
    void* ret = (void*)wglGetProcAddress(function);
    
    return ret;
}

} // wdk
