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
    // OpenGL framebuffer configuration
    class config : noncopyable
    {
    public:
        enum class backbuffer {
            dont_care, single_buffer, double_buffer

        };

        // framebuffer attributes. when using visualid or configid
        // other attributes are ignored. use 0 for don't care
        struct attributes {
            uint_t red_size;
            uint_t green_size;
            uint_t blue_size;
            uint_t alpha_size;
            uint_t depth_size;
            uint_t visualid;
            uint_t configid;

            backbuffer backbuf;

            struct {
                bool window;
                bool pbuffer;
                bool pixmap;
            } surfaces;
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

        // todo: static query functions
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

} // wdk

