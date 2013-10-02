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

#include <memory>
#include "utility.h"
#include "context.h"
#include "config.h"
#include "surface.h"

namespace wdk
{
    // *** NOTES ABOUT TEARING DOWN THE RENDERING CONTEXT PROPERLY. ***

    // the order of tearing down the rendering context properly
    // without leaving dangling handles behind (and then dumping core on X11 error)
    // means that first we have to set null for surface in the context
    // (since it depends on the surface), then dispose the surface
    // (since it depends on the buffer (window) handle) and then finally close the buffer (window)

    class window;
    class pixmap;

    // a wrapper to quickly setup opengl config, context and surface
    class opengl : noncopyable
    {
    public:
        opengl(const config::attributes& attrs) : config_(attrs), context_(config_)
        {}
        opengl() : context_(config_)
        {}
       ~opengl() 
        {
            if (surface_)
                detach();
        }

        // attach the given renderable as the new rendering target
        template<typename Renderable>
        void attach(Renderable& target)
        {
            context_.make_current(nullptr);
            surface_.reset(new surface(config_, target));
            context_.make_current(surface_.get());
        }

        // detach the currently set renderable from the rendering context.
        void detach()
        {
            context_.make_current(nullptr);
            surface_->dispose();
        }

        void swap()
        {
            context_.swap_buffers();
        }
        uint_t visualid() const
        {
            return config_.visualid();
        }
    private:
        config  config_;
        context context_;
        std::unique_ptr<surface> surface_;
    };

} // wdk