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
#include <wdk/utility.h>
#include <wdk/types.h>
#include "types.h"

namespace wdk
{
    // OpenGL framebuffer configuration
    class config : noncopyable
    {
    public:
        enum class multisampling {
            none,
            msaa4,
            msaa8,
            msaa16
        };

        // framebuffer attributes. when using visualid or configid
        // other attributes are ignored. use 0 for don't care
        struct attributes {
            // color buffer red bits
            uint_t red_size;

            // color buffer green bits
            uint_t green_size;

            // color buffer blue bits
            uint_t blue_size;

            // color buffer alpha bits
            uint_t alpha_size;

            // depth buffer bits
            uint_t depth_size;

            // stencil buffer bits
            uint_t stencil_size;

            // specific visual id by which configuration is chosen.
            uint_t visualid;

            // specific configuration which is to be used.
            uint_t configid;

            // double buffered or not. you'll generally want this to be true.
            bool double_buffer;

            // use sRGB frame buffer. 
            // see EXt_framebuffer_sRGB.txt.
            // this might not always be available, but depends on the driver.
            // remember to glEnable(FRAMEBUFFER_SRGB_EXT)
            // https://www.opengl.org/registry/specs/EXT/framebuffer_sRGB.txt            
            bool srgb_buffer;

            // possible rendering surfaces.
            struct {
                // window backed surface. Normal choice.
                bool window;  

                // offscreen rendering surface.
                bool pbuffer; 

                // window system provided surface such as a bitmap or a pixmap.
                // Note that this might not always be available or might be very
                // slow to render into.
                bool pixmap;  
            } surfaces;

            // multisample antialising. defaults to none.
            // remember to glEnable(GL_MULTISAMPLE)
            // https://www.opengl.org/registry/specs/ARB/multisample.txt
            multisampling sampling;
        };

        // some predefined attributes.
        static attributes DONT_CARE;
        static attributes DEFAULT;

        // create new config with the given attributes.
        // throws an exception if now such configuration is available.
        config(const attributes& attrs = config::DEFAULT);

       ~config();

        // get the visualid
        uint_t visualid() const;

        // get the config id
        uint_t configid() const;

        gl_config_t handle() const;

        bool srgb_buffer() const;

        // todo: static query functions
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

} // wdk

