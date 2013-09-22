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
#include "window.h"
#include "display.h"
#include "config.h"
#include "context.h"
#include "surface.h"

namespace wdk
{
    // wrapper around window, context and surface to quickly
    // setup a window for gl rendering
    class glwindow : noncopyable
    {
    public:
        glwindow(const display& disp) : display_(disp), window_(new window(disp))
        {
        }
       ~glwindow()
        {
            close();
        }

        // create window and context with the given framebuffer configuration
        void create(const window_params& how, const config& conf)
        {
            wdk::window_params p = how;
            p.visualid = conf.visualid();

            window_->create(p);

            surface_.reset(new surface(display_, conf, *window_));
            context_.reset(new context(display_, conf));
            context_->make_current(surface_.get());
        }

        // create window and context with default framebuffer configuration
        void create(const window_params& how)
        {
            wdk::config conf(display_);

            create(how, conf);
        }

        void swap_buffers()
        {
            context_->swap_buffers();
        }

        // close the window. after the window is closed there's rendering
        // context for the current thread.
        void close()
        {
            if (!window_->exists())
                return;
            // the order of tearing down the rendering context properly
            // without leaving dangling handles behind (and then dumping core on X11 error)
            // means that first we have to set null for surface in the context 
            // (since it depends on the surface), then dispose the surface 
            // (since it depends on the window handle) and then finally close the window
            context_->make_current(nullptr);

            surface_->dispose();

            window_->close();
        }

        bool exists() const
        {
            return window_->exists();
        }

        bool dispatch(const event& ev) const
        {
            return window_->dispatch(ev);
        }

        // get renderable (window inside area) surface width
        uint_t surface_width() const
        {
            return window_->surface_width();
        }

        // get renderable (window inside area) surface height
        uint_t surface_height() const
        {
            return window_->surface_height();
        }

        // get window width including title bar, borders etc.
        uint_t width() const
        {
            return window_->width();
        }

        // get window height including title bar, borders etc.
        uint_t height() const
        {
            return window_->height();
        }

        template<typename T>
        void set_listener(T& listener)
        {
            connect(*window_, listener);
        }

    private:
        const display& display_;

        std::unique_ptr<context> context_;
        std::unique_ptr<window> window_;
        std::unique_ptr<surface> surface_;
    };

} // wdk


