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

#include "wdk/types.h"
#include "wdk/opengl/types.h"

namespace wdk
{
    class Window;
    class Pixmap;
    class Config;

    // OpenGL rendering surface can be constructed for a window
    // system created object such as Window or Pixmap or
    // it can be created in a headless fashion where a OpenGL implementation
    // provided rendering surface is used.
    class Surface
    {
    public:
        // Create a rendering surface that renders to the given window.
        Surface(const Config& conf, const Window& win);

        // Create a rendering surface that renders to the given pixmap.
        Surface(const Config& conf, const Pixmap& px);

        // Create an offscreen width x height px rendering surface.
        Surface(const Config& conf, uint_t width, uint_t height);

       ~Surface();

        // Get surface width.
        uint_t GetWidth() const;

        // Get surface height.
        uint_t GetHeight() const;

        // Get implementation specific native handle.
        // On GLX this is GLXDrawable.
        // On EGL this is EGLSurface.
        // On WGL this is HDC.
        gl_surface_t GetNativeHandle() const;

        // Destroy and dispose of the rendering surface.
        // After having called this the Surface is no longer
        // valid rendering target.
        void Dispose();
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    }; 

} // wdk
