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

#include "wdk/types.h"
#include "types.h"

namespace wdk
{
    // OpenGL framebuffer configuration. Normal OpenGL context creation
    // first decides on some configuration parameters such as stencil buffer
    // size, color buffer (rgb) sizes, multisampling etc.
    // Once the desired attributes are set a configuration that matches 
    // those attributes is selected. The Configuration then is used to 
    // create the OpenGL context and a compatible rendering surface
    // such as Window. 
    class Config 
    {
    public:
        // Multisample antialiasing is a method where instead of sampling
        // each fragment once for computing the pixel coverage when rasterizing
        // a triangle there are multiple samples per fragment. 
        // Finally the samples are combined in a "resolve" step to create the
        // final fragment color in the color buffer.
        // The larger the number of samples per fragment the larger the memory
        // requirements for the color buffer.
        // https://en.wikipedia.org/wiki/Multisample_anti-aliasing
        enum class Multisampling {
            // Multsampling is disabled. Only one sample per fragment.
            None,
            // 4 samples per fragment.
            MSAA4,
            // 8 samples per fragment.
            MSAA8,
            // 16 samples per fragment.
            MSAA16
        };

        // Possible attributes for config selection.
        // When using visualid or configid other attributes are ignored. 
        // Use 0 for don't care which omits the attribute from 
        // config selection. 
        struct Attributes {
            // color buffer red bits
            uint_t red_size = 0;

            // color buffer green bits
            uint_t green_size = 0;

            // color buffer blue bits
            uint_t blue_size = 0;

            // color buffer alpha bits
            uint_t alpha_size = 0;

            // depth buffer bits
            uint_t depth_size = 0;

            // stencil buffer bits
            uint_t stencil_size = 0;

            // specific configuration which is to be used.
            uint_t configid = 0;

            // double buffered or not. you'll generally want this to be true.
            bool double_buffer = true;

            // use sRGB frame buffer. 
            // see EXT_framebuffer_sRGB.txt.
            // this might not always be available, but depends on the driver.
            // remember to glEnable(FRAMEBUFFER_SRGB_EXT)
            // https://www.opengl.org/registry/specs/EXT/framebuffer_sRGB.txt            
            bool srgb_buffer = true;

            // possible rendering surfaces.
            struct {
                // window backed surface. Normal choice.
                bool window = true;

                // offscreen rendering surface.
                bool pbuffer = false;

                // window system provided surface such as a bitmap or a pixmap.
                // Note that this might not always be available or might be very
                // slow to render into.
                bool pixmap = false;
            } surfaces;

            // multisample antialising. defaults to none.
            // remember to glEnable(GL_MULTISAMPLE)
            // https://www.opengl.org/registry/specs/ARB/multisample.txt
            Multisampling sampling = Multisampling::None;
        };

        // some predefined attributes.
        static Attributes DONT_CARE;
        static Attributes DEFAULT;

        // create new config with the given attributes.
        // throws an exception if no such configuration is available.
        Config(const Attributes& attrs = Config::DEFAULT);
       ~Config();

        // Get the visualid that is used to identify compatible items.
        // The visual ID can then be used to create other compatible
        // objects such as Windows.
        uint_t GetVisualID() const;

        // Get the config id that uniquely identifies this config
        // in the underlying OpenGL implementation.
        uint_t GetConfigID() const;

        // Get the underlying native implementation specific config handle.
        // On GLX this is GLXFBConfig. 
        // On EGL this is EGLConfig 
        // on WGL this is PIXELFORMATDESCRIPTOR*
        gl_config_t GetNativeHandle() const;

        // Returns true if sRGB enabled, otherwise false.
        bool sRGB() const;

        // todo: static query functions
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

} // wdk

