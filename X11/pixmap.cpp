
#include <X11/Xlib.h>
#include <stdexcept>
#include <functional>
#include <cassert>
#include "../pixmap.h"
#include "error_handler.h"

namespace wdk
{

struct pixmap::impl {
    Pixmap handle;
    Display* disp;
    uint_t width;
    uint_t height;
    uint_t depth;
};

pixmap::pixmap(native_display_t disp, uint_t width, uint_t height, uint_t visualid)
{
    // todo: visualid??

    uint_t bit_depth = 32;

    assert(disp);
    assert(width && height);

    pimpl_.reset(new impl);
    pimpl_->handle = 0;
    pimpl_->disp   = disp;
    pimpl_->width  = 0;
    pimpl_->height = 0;
    pimpl_->depth  = 0;

    factory<Pixmap> px_factory(disp);

    Pixmap px = px_factory.create(std::bind(XCreatePixmap, std::placeholders::_1, RootWindow(disp, DefaultScreen(disp)), width, height, bit_depth));
    if (px_factory.has_error())
        throw std::runtime_error("failed to create pixmap");

    pimpl_->handle = px;
    pimpl_->width  = width;
    pimpl_->height = height;
    pimpl_->depth  = bit_depth;

#ifndef _NDEBUG
    uint_t w, h, d;
    int x, y;
    uint_t border;
    Window dummy;
    XGetGeometry(pimpl_->disp, px, &dummy, &x, &y, &w, &h, &border, &d);

    assert(w == width);
    assert(h == height);
    //assert(d == bit_depth);
#endif  
}

pixmap::pixmap(native_display_t disp, 
    uint_t width, uint_t height, const attributes& attr)
{
    
}

pixmap::~pixmap()
{
    XFreePixmap(pimpl_->disp, pimpl_->handle);
}

native_pixmap_t pixmap::handle() const
{
    return native_pixmap_t {pimpl_->handle};
}

native_display_t pixmap::display() const
{
    return pimpl_->disp;
}

uint_t pixmap::width() const
{
    return pimpl_->width;
}

uint_t pixmap::height() const
{
    return pimpl_->height;
}

uint_t pixmap::depth() const
{
    return pimpl_->depth;
}


} // wdk
