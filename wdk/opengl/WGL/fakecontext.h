
#pragma once

#include <windows.h>
#include <stdexcept>
#include <cassert>
#include <memory>
#include <wdk/utility.h>

#pragma comment(lib, "opengl32.lib")

namespace wdk {
    class config;
} // wdk

namespace wgl
{
    // on WGL we need this dummy trampoline context that we can 
    // create in order to query the runtime for functions to create
    // the "real" context.
    class FakeContext : wdk::noncopyable
    {
    public:
        FakeContext(const PIXELFORMATDESCRIPTOR& conf)
        {
            WNDCLASSEX cls = {0};
            cls.cbSize = sizeof(cls);
            cls.style  = CS_OWNDC;
            cls.lpfnWndProc = DefWindowProc;
            cls.lpszClassName = TEXT("WDKFakeWindow");
            RegisterClassEx(&cls);
            m_hwnd = CreateWindow(TEXT("WDKFakeWindow"), TEXT(""), 
                WS_POPUP, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
            m_hdc  = GetDC(m_hwnd);    

            const int PixelFormat = ChoosePixelFormat(m_hdc, &conf);
            if (!SetPixelFormat(m_hdc, PixelFormat, &conf))
                throw std::runtime_error("FakeContext: SetPixelFormat failed"); 
            m_hgl = wglCreateContext(m_hdc);
            if (!m_hgl)
                throw std::runtime_error("FakeContext: Create context failed.");
            m_pixel_format = PixelFormat;
        }
       ~FakeContext() 
        {
            BOOL ret = TRUE;
            ret = wglDeleteContext(m_hgl);
            assert(ret == TRUE);
            ret = ReleaseDC(m_hwnd, m_hdc);
            assert(ret == TRUE);
            ret = DestroyWindow(m_hwnd);
            assert(ret == TRUE);
            (void)ret;
        }

        void* resolve(const char* function) const
        {
            assert(function);
            const auto hgl = wglGetCurrentContext();
            const auto hdc = wglGetCurrentDC();

            if (wglMakeCurrent(m_hdc, m_hgl) == FALSE)
                throw std::runtime_error("failed to set context current");
            
            void* ret = (void*)wglGetProcAddress(function);
            if (wglMakeCurrent(hdc, hgl) == FALSE)
                throw std::runtime_error("failed to restore context");

            return ret;
        }
        template<typename FunctionPointerT>
        FunctionPointerT resolve(const char* function) const
        {
            return reinterpret_cast<FunctionPointerT>(resolve(function));
        } 

        HDC getDC() const 
        { return m_hdc; }

        HWND getHWND() const 
        { return m_hwnd; }

    private:
        HWND  m_hwnd;
        HDC   m_hdc;
        HGLRC m_hgl;
        int   m_pixel_format;
    };

    void stashFakeContext(const wdk::config* config, std::shared_ptr<FakeContext> ctx);
    void fetchFakeContext(const wdk::config* config, std::shared_ptr<FakeContext>& ctx);
    void eraseFakeContext(const wdk::config* config);


} // namespace