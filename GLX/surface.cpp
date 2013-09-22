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
#include "../surface.h"
#include "../display.h"
#include "../config.h"
#include "../window.h"
#include "../pixmap.h"
#include "../X11/error_handler.h"

namespace {
    enum class surface_type { window, pixmap, pbuffer };
} //

namespace wdk
{
struct surface::impl {
    Display*     dpy;
    GLXDrawable  surface;
    surface_type type;
};

surface::surface(const display& disp, const config& conf, const window& win)
{
    pimpl_.reset(new impl);

    factory<GLXDrawable> fac(disp.handle());

    GLXDrawable surface = fac.create(std::bind(glXCreateWindow, std::placeholders::_1, 
        conf.handle(), win.handle(), nullptr));
    if (fac.has_error())
        throw std::runtime_error("create window surface failed");

    pimpl_->dpy     = disp.handle();
    pimpl_->surface = surface;
    pimpl_->type    = surface_type::window;
}

surface::surface(const display& disp, const config& conf, const pixmap& px)
{
    pimpl_.reset(new impl);

    factory<GLXDrawable> fac(disp.handle());

    GLXDrawable surface = fac.create(std::bind(glXCreatePixmap, std::placeholders::_1, 
        conf.handle(), px.handle(), nullptr));
    if (fac.has_error())
        throw std::runtime_error("create pixmap surface failed");

    pimpl_->dpy     = disp.handle();
    pimpl_->surface = surface;
    pimpl_->type    = surface_type::pixmap;
}

surface::surface(const display& disp, const config& conf, uint_t width, uint_t height)
{
    pimpl_.reset(new impl);

    factory<GLXDrawable> fac(disp.handle());

    const int attrs[] = {
        GLX_PBUFFER_WIDTH, (int)width,
        GLX_PBUFFER_HEIGHT, (int)height,
        None
    };

    GLXDrawable surface = fac.create(std::bind(glXCreatePbuffer, std::placeholders::_1, 
        conf.handle(), attrs));
    if (fac.has_error())
        throw std::runtime_error("create offscreen surface failed");

    pimpl_->dpy     = disp.handle();
    pimpl_->surface = surface;
    pimpl_->type    = surface_type::pbuffer;
}

surface::~surface()
{
    dispose();
}

uint_t surface::width() const
{
    uint_t width = 0;

    glXQueryDrawable(pimpl_->dpy, pimpl_->surface, GLX_WIDTH, &width);

    return width;
}

uint_t surface::height() const
{
    uint_t height = 0;

    glXQueryDrawable(pimpl_->dpy, pimpl_->surface, GLX_HEIGHT, &height);

    return height;
}

gl_surface_t surface::handle() const
{
    return gl_surface_t {pimpl_->surface};
}

void surface::dispose()
{
    if (!pimpl_->surface)
        return;

    switch (pimpl_->type)
    {
        case surface_type::window:
            glXDestroyWindow(pimpl_->dpy, pimpl_->surface);
            break;

        case surface_type::pbuffer:
            glXDestroyPbuffer(pimpl_->dpy, pimpl_->surface);
            break;

        case surface_type::pixmap:
           glXDestroyPixmap(pimpl_->dpy, pimpl_->surface);
           break;
    }    
    pimpl_->surface = 0;
}

} // wdk
