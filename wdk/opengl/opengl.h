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

    class Window;
    class Pixmap;

    // Wrapper to quickly setup opengl config, context and surface
    // By default the type of context depends on the platform being used.
    // When linking against the "desktop libraries", i.e. WGL on Windows
    // and GLX on Linux a desktop OpenGL context is created. When linking
    // against EGL a "mobile" OpenGL ES context is created. However when
    // the right extensions are present (WGL_EXT_create_context_es2_profile,
    // GLX_EXT_create_context_es2_profile) it's also possible to create an
    // OpenGL ES context while linking against the desktop libraries.
    class OpenGL
    {
    public:
        // Create platform's native context with default version but with specific attributes.
        OpenGL(const Config::Attributes& attrs) : config_(attrs), context_(config_)
        {}

        // Create platform's native context with specific version and attributes.
        OpenGL(const Config::Attributes& attrs, int major_version, int minor_version, bool debug)
            : config_(attrs), context_(config_, major_version, minor_version, debug)
        {}
        // Create non-native context with specific version and attributes.
        // Requires the right platform extensions in order to work.
        OpenGL(const Config::Attributes& attrs, Context::Type type,
               int major_version, int minor_version, bool debug)
            : config_(attrs), context_(config_, major_version, minor_version, debug, type)
        {}

        // Create platform's native context with specific version and with default attributes
        OpenGL(int major_version, int minor_version, bool debug)
            : context_(config_, major_version, minor_version, debug)
        {}
        // Create non-native context with specific version and with default attributes.
        // Requires the right platform extensions in order to work.
        OpenGL(Context::Type type, int major_version, int minor_version, bool debug)
            : context_(config_, major_version, minor_version, debug, type)
        {}

        // Create default context version with default attributes.
        OpenGL() : context_(config_)
        {}

       ~OpenGL()
        {
            if (surface_)
                Detach();
        }

        // Create a new Surface for the given render target and then
        // attach the surface to the context.
        template<typename RenderTarget>
        void Attach(RenderTarget& target)
        {
            context_.MakeCurrent(nullptr);
            surface_.reset(new Surface(config_, target));
            context_.MakeCurrent(surface_.get());
        }

        // Attach the given surface to the context as the render target.
        void Attach(Surface& surf)
        {
            context_.MakeCurrent(nullptr);
            surface_.reset();
            context_.MakeCurrent(&surf);
        }

        // Make the context current for the calling thread
        // with a surface that was created previously through
        // a call to Attach.
        void MakeCurrent()
        {
            context_.MakeCurrent(surface_.get());
        }

        // Detach and dispose he currently set renderable from the rendering context.
        // After having detached the rendering surface the context can no longer
        // be used to render.
        void Detach()
        {
            context_.MakeCurrent(nullptr);
            if (surface_)
                surface_->Dispose();
        }

        // Swap the back/front buffers. See Context::SwapBuffers.
        void SwapBuffers()
        {
            context_.SwapBuffers();
        }

        // Set the swap interval. See Context::SetSwapInterval.
        bool SetSwapInterval(int interval)
        {
            return context_.SetSwapInterval(interval);
        }

        // Get the Visual ID. See Config::GetVisualID
        uint_t GetVisualID() const
        {
            return config_.GetVisualID();
        }
        // Get the config ID. See Config::GetConfigID
        uint_t GetConfigID() const
        {
            return config_.GetConfigID();
        }

        // Get config object.
        const Config& GetConfig() const
        {
            return config_;
        }

        // Resolve OpenGL entry point. See Context::Resolve
        void* Resolve(const char* function) const
        {
            return context_.Resolve(function);
        }

        // Get the native underlying OpenGL context handle.
        void* GetNativeHandle() const
        {
            return context_.GetNativeHandle();
        }
    private:
        Config  config_;
        Context context_;
        std::unique_ptr<Surface> surface_;
    };

} // wdk