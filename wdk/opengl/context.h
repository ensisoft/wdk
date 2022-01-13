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

namespace wdk
{
    class Config;
    class Surface;

    // OpenGL rendering context. Any application that wants to
    // perform OpenGL rendering will need to create a "context"
    // for each thread that wants to perform rendering.
    // The context is the "main" object that contains all the current
    // graphics state.
    // The context is always "implicit" in the API state i.e. instead
    // of it getting passed to the OpenGL API calls, it's just set as the
    // "current context" for the calling thread.
    class Context
    {
    public:
        // Create a rendering context compatible with the given
        // configuration and with default GL version. (GL 3.0, GLES 2.0)
        Context(const Config& conf);

        // Create a rendering context compatible with the given
        // configuration and with a specific GL version.
        // If debug is true the context is created as a debug context if possible.
        Context(const Config& conf, int major_version, int minor_version, bool debug);

        // Preferred context type
        enum Type {
            OpenGL, // Open GL
            OpenGL_ES, // OpenGL ES
        };
        // Create a rendering context compatible with the given
        // configuration and with a specific GL version.
        // if debug is true the context is created as a debug context if possible.
        // Using the ContextType one can ask the backend to create a "non native"
        // context.
        // By default WGL and GLX will create a OpenGL context and EGL will
        // create a OpenGL ES context. In other words if you're linking against
        // the "desktop libraries" by default you'll get an Open GL context
        // and if you're linking against EGL you'll get an Open GL ES context.
        // However on some platforms it's possible to use an extension to
        // use the desktop library to create an OpenGL ES context.
        // On Windows (WGL) This requires WGL_EXT_create_context_es2_profile and
        // on Linux (GLX) this requires GLX_EXT_create_context_es2_profile
        // Currently it's not possible to use EGL to create a desktop context.
        Context(const Config& conf, int major_version, int minor_version, bool debug,
            Type requested_type);

        // dtor
       ~Context();

        // Make this context the current context for the calling thread.
        // The Surface can be a nullptr in which case the context is
        // detached from any previous rendering surface and no further
        // rendering is possible until a new surface object is provided.
        void MakeCurrent(Surface* surf);

        // Typical OpenGL applications use a so-called "double buffered"
        // rendering surfaces to avoid a problem where the user would be
        // displayed partially rendered image. Instead, one buffer is being
        // displayed to the user while the other (so called back buffer)
        // buffer is used as the rendering target for rendering the next
        // image to be displayed. Once all the rendering commands have been
        // executed the buffers are "swapped" i.e. the back buffer is
        // displayed and the old front buffer becomes the new back buffer.
        void SwapBuffers();

        // Has direct rendering or not. Some window systems may not provide
        // direct access to the rendering hardware. (X11 Remoting).
        bool HasDRI() const;

        // Set the minimum number of display surface swaps before changes
        // to the current surface are synced to the video display, i.e.
        // before SwapBuffers actually displays the current rendering surface.
        // When interval = 0, this effectively turns off display sync
        // When interval = 1, rendering is synced to display sync (i.e.
        // one buffer swap per each display sync)
        // This requires the implementation to provide the appropriate
        // extensions: GLX_EXT_swap_control on GLX (Linux) and
        // WGL_EXT_swap_control on WGL. (Windows).
        // Returns true if setting was successful or false if anything failed.
        // Before this can be called the context has to have been made
        // current with a non-null surface object.
        // After that the setting will take effect from the next SwapBuffers on.
        bool SetSwapInterval(int interval);

        // Resolve an OpenGL entry point to a function pointer.
        // Note that the returned function pointers *may* be context
        // specific depending on the particular implementation
        // and configuration. For example on Windows the pointers
        // may change if the context use different pixel formats.
        // Never assume that one set of pointers are applicable
        // to another context.
        // Returns nullptr if no such function is available.
        void* Resolve(const char* function) const;

        // Get the native underlying OpenGL context handle.
        void* GetNativeHandle() const;
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

} // wdk

