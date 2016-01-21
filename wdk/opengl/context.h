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

namespace wdk
{
    class config;
    class surface;

    // OpenGL rendering context.
    class context : noncopyable
    {
    public:
        // create a rendering context compatible with the given
        // configuration and with default GL version. (GL 3.0, GLES 2.0)
        context(const config& conf);

        // create a rendering context compatible with the given
        // configuration and with a specific GL version.
        // if debug is true the context is created as a debug context if possible.
        context(const config& conf, int major_version, int minor_version, bool debug);

        // dtor
       ~context();

        void make_current(surface* surf);

        // swap back and front buffers of the current surface
        void swap_buffers();

        // has direct rendering or not
        bool has_dri() const;

        // resolve (an extension) function
        static void* resolve(const char* function);
        
    private:
        struct impl;
        
        std::unique_ptr<impl> pimpl_;
    };

} // wdk

