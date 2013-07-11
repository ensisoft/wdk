

#pragma once

#include <memory>
#include "types.h"
#include "utility.h"

namespace wdk
{
    // OpenGL rendering context
    class context : noncopyable
    {
    public:
        // create a context using default attributes.
        // on succesful return the context is the new current context
        // and any previous current context state is lost.
        context(native_display_t disp);

        // create a context using a list of attributes.
        // the list of attributes should not be empty but at least contain
        // the terminating attribute. I.e. EGL_NONE, None (for GLX) etc.
        // on succesful return the context is the new current context
        // and any previous current context state is lost.
        context(native_display_t disp, const int_t* attrs);
        
        // dtor
       ~context();

        // make the given window the current rendering
        // window for this context
        void make_current(native_window_t window);

        // swap back and front buffers
        void swap_buffers();

        // get visualid for the chosen configuration
        uint_t visualid() const;

        // has direct rendering or not
        bool has_dri() const;
        
        // resolve an extension function
        void* resolve(const char* function);
    private:
        void init(native_display_t disp, const int_t* attrs);

        struct impl;
        
        std::unique_ptr<impl> pimpl_;
    };

} // wdk

