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
#include "types.h"
#include "utility.h"

namespace wdk
{
    // OpenGL rendering context.
    class context : noncopyable
    {
    public:
        // context and color buffer attributes.
        struct attributes {
            uint_t major_version;
            uint_t minor_version;
            uint_t red_size;
            uint_t green_size;
            uint_t blue_size;
            uint_t alpha_size;
            uint_t depth_size;
            bool   render_window;
            bool   render_pixmap;
            bool   doublebuffer;

            // when using visualid to create the context other color buffer
            // configuration attributes are ignored. 
            uint_t visualid; 
            // construct default attributes as follows:
            // - Desktop: GL 3.0, red 8, green 8, blue 8, alpha 8, depth 16, render_window, doublebuffer
            // - Mobile: GL ES 2.0, red 8, green 8, blue 8, alpha 8, depth 8, render_window, doublebuffer
            attributes();
        };

        // create a context optionally passing customized context attributes
        // for the context and config creation. on succesful return the context 
        // is the new current context for the calling thread.
        context(native_display_t disp, const attributes& attrs = attributes());

        // dtor
       ~context();

        // render to a window surface
        void make_current(native_window_t window);

        // render to a pixmap surface
        void make_current(native_pixmap_t pixmap);
        
        // swap back and front buffers of the current surface
        void swap_buffers();

        // get visualid of the chosen configuration.
        // the visual id can be used to create rendering surfaces (window/pixmap)
        // that are compatible with this rendering context
        uint_t visualid() const;

        // has direct rendering or not
        bool has_dri() const;
        
        // resolve (an extension) function
        static void* resolve(const char* function);
    private:
        struct impl;
        
        std::unique_ptr<impl> pimpl_;
    };

} // wdk

