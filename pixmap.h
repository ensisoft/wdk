

#pragma once

#include <memory>
#include "utility.h"
#include "types.h"

namespace wdk
{
    // 
    class pixmap : noncopyable
    {
    public:
        struct attributes {
            uint_t red_size;
            uint_t green_size;
            uint_t blue_size;
            uint_t alpha_size;
        };

        // construct a pixmap compatible with the given visualid
        pixmap(native_display_t disp, 
            uint_t width, uint_t height, uint_t visualid);

        pixmap(native_display_t disp,
            uint_t width, uint_t height, 
            const attributes& attr = attributes());

       ~pixmap();

        native_pixmap_t handle() const;

        native_display_t display() const;

        uint_t width() const;
        uint_t height() const;
        uint_t depth() const;
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
   };

} // wdk
