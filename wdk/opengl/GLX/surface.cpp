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

#include <GL/glx.h>

#include <functional>
#include <stdexcept>

#include "wdk/X11/errorhandler.h"
#include "wdk/system.h"
#include "wdk/window.h"
#include "wdk/pixmap.h"
#include "wdk/opengl/surface.h"
#include "wdk/opengl/config.h"

#define X11_None 0L

namespace {
    enum class surface_type { window, pixmap, pbuffer };
} //

namespace wdk
{
struct Surface::impl {
    GLXDrawable  surface;
    surface_type type;
};

Surface::Surface(const Config& conf, const Window& win)
{
    pimpl_.reset(new impl);

    factory<GLXDrawable> fac(GetNativeDisplayHandle());

    GLXDrawable surface = fac.create(std::bind(glXCreateWindow, std::placeholders::_1, 
        conf.GetNativeHandle(), win.GetNativeHandle(), nullptr));
    if (fac.has_error())
        throw std::runtime_error("create window surface failed");

    pimpl_->surface = surface;
    pimpl_->type    = surface_type::window;
}

Surface::Surface(const Config& conf, const Pixmap& px)
{
    pimpl_.reset(new impl);

    factory<GLXDrawable> fac(GetNativeDisplayHandle());

    GLXDrawable surface = fac.create(std::bind(glXCreatePixmap, std::placeholders::_1, 
        conf.GetNativeHandle(), px.GetNativeHandle(), nullptr));
    if (fac.has_error())
        throw std::runtime_error("create pixmap surface failed");

    pimpl_->surface = surface;
    pimpl_->type    = surface_type::pixmap;
}

Surface::Surface(const Config& conf, uint_t width, uint_t height)
{
    pimpl_.reset(new impl);

    factory<GLXDrawable> fac(GetNativeDisplayHandle());

    const int attrs[] = {
        GLX_PBUFFER_WIDTH, (int)width,
        GLX_PBUFFER_HEIGHT, (int)height,
        X11_None
    };

    GLXDrawable surface = fac.create(std::bind(glXCreatePbuffer, std::placeholders::_1, conf.GetNativeHandle(), attrs));
    if (fac.has_error())
        throw std::runtime_error("create offscreen surface failed");

    pimpl_->surface = surface;
    pimpl_->type    = surface_type::pbuffer;
}

Surface::~Surface()
{
    Dispose();
}

uint_t Surface::GetWidth() const
{
    Display* d = GetNativeDisplayHandle();

    uint_t width = 0;

    glXQueryDrawable(d, pimpl_->surface, GLX_WIDTH, &width);

    return width;
}

uint_t Surface::GetHeight() const
{
    Display* d = GetNativeDisplayHandle();

    uint_t height = 0;

    glXQueryDrawable(d, pimpl_->surface, GLX_HEIGHT, &height);

    return height;
}

gl_surface_t Surface::GetNativeHandle() const
{
    return gl_surface_t {pimpl_->surface};
}

void Surface::Dispose()
{
    if (!pimpl_->surface)
        return;

    Display* d = GetNativeDisplayHandle();

    switch (pimpl_->type)
    {
        case surface_type::window:
            glXDestroyWindow(d, pimpl_->surface);
            break;

        case surface_type::pbuffer:
            glXDestroyPbuffer(d, pimpl_->surface);
            break;

        case surface_type::pixmap:
           glXDestroyPixmap(d, pimpl_->surface);
           break;
    }    
    pimpl_->surface = 0;
}

} // wdk
