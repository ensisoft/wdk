
#include <windows.h>
#include <cassert>
#include <stdexcept>
#include <map>
#include "context.h"

namespace {
    struct rendering_context {
        HDC   hdc;
        HGLRC ctx;
    };
} // namespace

namespace wdk
{
struct context::impl {
    std::map<HWND, rendering_context> contexts;
    HDC current;
};

context::context(native_display_t disp)
{
    init(disp, nullptr);
}

context::context(native_display_t disp, const int_t* attrs)
{
    init(disp, attrs);
}

context::~context()
{
    auto& map = pimpl_->contexts;
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        assert(it->second.hdc);
        assert(it->second.ctx);
        wglDeleteContext(it->second.ctx);
        ReleaseDC(it->first, it->second.hdc);
    }
}

void context::make_current(native_window_t window)
{
    if (window == wdk::NULL_WINDOW)
    {
        wglMakeCurrent(NULL, NULL);
        return;
    }
    auto& map = pimpl_->contexts;

    auto it = map.find(window);

    if (it == map.end())
    {
        struct auto_dc {
            HWND wnd;
            HDC  hdc;
            auto_dc(HWND window) : wnd(window) {
                hdc = GetDC(window);
                if (!hdc)
                    throw std::runtime_error("get device context failed");
            }
           ~auto_dc() {
               if (hdc)
                   ReleaseDC(wnd, hdc);
           }
           void release() {
               wnd = NULL;
               hdc = NULL;
           }
        } dc(window);
        
        PIXELFORMATDESCRIPTOR desc = {};
        desc.nSize      = sizeof(desc);
        desc.nVersion   = 1;
        desc.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        desc.iPixelType = PFD_TYPE_RGBA;
        desc.cColorBits = 32;
        desc.cRedBits   = 8;
        desc.cGreenBits = 8;
        desc.cBlueBits  = 8;
        desc.cAlphaBits = 8;
        desc.cDepthBits = 16;
        
        int pixelformat = ChoosePixelFormat(dc.hdc, &desc);
        if (!pixelformat)
            throw std::runtime_error("choose pixel format failed");

        if (!SetPixelFormat(dc.hdc, pixelformat, &desc))
            throw std::runtime_error("set pixel format failed");

        struct auto_ctx {
            HGLRC ctx;
            auto_ctx(HDC hdc) {
                ctx = wglCreateContext(hdc);
                if (!ctx)
                    throw std::runtime_error("create context failed");
            }
           ~auto_ctx() {
               if (ctx)
                   wglDeleteContext(ctx);
           }
           void release() {
               ctx = NULL;
           }
        } ctx(dc.hdc);
        
        if (!wglMakeCurrent(dc.hdc, ctx.ctx))
            throw std::runtime_error("make current failed");

        rendering_context rc = {dc.hdc, ctx.ctx};
        map.insert(std::make_pair(window, rc));

        pimpl_->current = dc.hdc;

        dc.release();
        ctx.release();
    }
    else
    {
        assert(it->second.hdc);
        assert(it->second.ctx);
        if (!wglMakeCurrent(it->second.hdc, it->second.ctx))
            throw std::runtime_error("make current failed");
        pimpl_->current = it->second.hdc;
    }
}

void context::swap_buffers()
{
    assert(pimpl_->current && "context has no valid surface/window. did you forget to call make_current?");
    
    SwapBuffers(pimpl_->current);
}

uint_t context::visualid() const
{
    return 0;
}

bool context::has_dri() const
{
    return true;
}

void* context::resolve(const char* function)
{
    assert(function && "null function name");
    
    void* ret = (void*)wglGetProcAddress(function);
    
    return ret;
}

void context::init(native_display_t disp, const int_t* attrs)
{
    pimpl_.reset(new impl);
    pimpl_->current = NULL;
}


} // wdk
