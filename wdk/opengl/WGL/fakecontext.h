
#pragma once

#include <windows.h>

#include <stdexcept>
#include <cassert>
#include <memory>

#include "wdk/utility.h"

#pragma comment(lib, "opengl32.lib")

namespace wdk {
    class Config;
} // wdk

namespace wgl
{
    // on WGL we need this dummy trampoline context that we can 
    // create in order to query the runtime for functions to create
    // the "real" context.
    class FakeContext
    {
    public:
        FakeContext(const FakeContext&) = delete;

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
            m_hdc  = ::GetDC(m_hwnd);    

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

        void* Resolve(const char* function) const
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
        FunctionPointerT Resolve(const char* function) const
        {
            return reinterpret_cast<FunctionPointerT>(Resolve(function));
        } 

        HDC GetDC() const 
        { return m_hdc; }

        HWND GetHWND() const 
        { return m_hwnd; }

        FakeContext& operator=(const FakeContext&) = delete;

    private:
        HWND  m_hwnd = NULL;
        HDC   m_hdc  = NULL;
        HGLRC m_hgl  = NULL;
        int   m_pixel_format = 0;
    };

    void StashFakeContext(const wdk::Config* config, std::shared_ptr<FakeContext> ctx);
    void FetchFakeContext(const wdk::Config* config, std::shared_ptr<FakeContext>& ctx);
    void EraseFakeContext(const wdk::Config* config);


} // namespace