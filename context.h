

#pragma once

#include <memory>
#include "types.h"
#include "utility.h"

namespace wdk
{
    // OpenGL rendering context.
    class context : noncopyable
    {
    public:
        // context and color buffer attributes.
        struct attributes {
            uint_t major_version;
            uint_t minor_version;
            uint_t red_size;
            uint_t green_size;
            uint_t blue_size;
            uint_t alpha_size;
            uint_t depth_size;
            bool   render_window;
            bool   render_pixmap;
            bool   doublebuffer;

            // when using visualid to create the context other color buffer
            // configuration attributes are ignored. 
            uint_t visualid; 
            // construct default attributes as follows:
            // - Desktop: GL 3.0, red 8, green 8, blue 8, alpha 8, depth 16, render_window, doublebuffer
            // - Mobile: GL ES 2.0, red 8, green 8, blue 8, alpha 8, depth 8, render_window, doublebuffer
            attributes();
        };

        // create a context optionally passing customized context attributes
        // for the context and config creation. on succesful return the context 
        // is the new current context for the calling thread.
        context(native_display_t disp, const attributes& attrs = attributes());

        // dtor
       ~context();

        // render to a window surface
        void make_current(native_window_t window);

        // render to a pixmap surface
        void make_current(native_pixmap_t pixmap);
        
        // swap back and front buffers of the current surface
        void swap_buffers();

        // get visualid of the chosen configuration.
        // the visual id can be used to create rendering surfaces (window/pixmap)
        // that are compatible with this rendering context
        uint_t visualid() const;

        // has direct rendering or not
        bool has_dri() const;
        
        // resolve (an extension) function
        static void* resolve(const char* function);
    private:
        struct impl;
        
        std::unique_ptr<impl> pimpl_;
    };

} // wdk

