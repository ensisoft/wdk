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

#include <stdexcept>
#include "egldisplay.h"

namespace wdk
{

EGLDisplay egl_init(native_display_t disp)
{
    // initialize the EGL display. this is a bit of a hack
    // and won't work if there ever is more than a single
    // native display handle. 
    struct egl 
    { 
        EGLDisplay display;

        egl(native_display_t disp)
        {
#if defined(WINDOWS) || defined(_WIN32)
            display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#else
            display = eglGetDisplay(disp);
#endif
            if (!display)
                throw std::runtime_error("eglGetDisplay failed");

            EGLint major = 0;
            EGLint minor = 0;
            if (!eglInitialize(display, &major, &minor))
                throw std::runtime_error("eglInitialize failed");
        }
       ~egl()
        {
            eglTerminate(display);
        }
    };
    static egl init(disp);

    return init.display;
}

} // wdk