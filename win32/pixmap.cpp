
#include <windows.h>
#include <stdexcept>
#include <cassert>
#include "../pixmap.h"
#include "../utility.h"

namespace wdk
{
struct pixmap::impl {
    HBITMAP bmp;
    HDC     dpy;
    uint_t  width;
    uint_t  height;
    uint_t  depth;
};

pixmap::pixmap(native_display_t disp, uint_t width, uint_t height, uint_t visualid)
{
    assert(disp);
    assert(width && height);
    assert(visualid);

    auto hdc = make_unique_ptr(CreateCompatibleDC(disp), DeleteDC);
    if (!hdc.get())
        throw std::runtime_error("create compatible dc failed");

    PIXELFORMATDESCRIPTOR desc = {0};
    DescribePixelFormat(hdc.get(), visualid, sizeof(desc), &desc);
    if (!SetPixelFormat(hdc.get(), visualid, &desc))
        throw std::runtime_error("set pixelformat failed");

    auto bmp = make_unique_ptr(CreateCompatibleBitmap(hdc.get(), width, height), DeleteObject);
    if (!bmp.get())
        throw std::runtime_error("create bitmap failed");

    pimpl_.reset(new impl);
    pimpl_->bmp    = bmp.release();
    pimpl_->dpy    = disp;
    pimpl_->width  = width;
    pimpl_->height = height;
    pimpl_->depth  = desc.cColorBits;
}

pixmap::pixmap(native_display_t disp, uint_t width, uint_t height, const attributes& attr)
{

}

pixmap::~pixmap()
{
    DeleteObject(pimpl_->bmp);
}

native_pixmap_t pixmap::handle() const
{
    return pimpl_->bmp;
}

native_display_t pixmap::display() const
{
    return pimpl_->dpy;
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
