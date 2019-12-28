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

// for clang in SublimeText2
#ifndef WDK_MOBILE
#  define WDK_MOBILE
#endif

#include <EGL/egl.h>

#include <stdexcept>
#include <vector>
#include <cassert>

#include "wdk/system.h"
#include "wdk/opengl/config.h"
#include "egldisplay.h"

namespace {
    void set_if(std::vector<wdk::uint_t>& v, wdk::uint_t attr, wdk::uint_t value)
    {
        if (value)
        {
            v.push_back(attr);
            v.push_back(value);
        }
    }

} // namespace

namespace wdk
{
Config::Attributes GetDefaultAttrs()
{
    Config::Attributes attrs;
    attrs.red_size = 8;
    attrs.green_size = 8;
    attrs.blue_size = 8;
    attrs.alpha_size = 8;
    attrs.depth_size = 8;
    attrs.stencil_size = 8;
    attrs.double_buffer = true;
    attrs.srgb_buffer = false;
    attrs.surfaces.window = true;
    attrs.sampling = Config::Multisampling::None;
    return attrs;
}

Config::Attributes Config::DONT_CARE;
Config::Attributes Config::DEFAULT = GetDefaultAttrs(); 

struct Config::impl {
    EGLDisplay   display;
    EGLConfig    config;
    uint_t       visualid;
    uint_t       configid;
    bool         srgb;
};

Config::Config(const Attributes& attrs) : pimpl_(new impl)
{
    pimpl_->display = egl_init(GetNativeDisplayHandle());

    std::vector<uint_t> criteria;

    set_if(criteria, EGL_RED_SIZE, attrs.red_size);
    set_if(criteria, EGL_GREEN_SIZE, attrs.green_size);
    set_if(criteria, EGL_BLUE_SIZE, attrs.blue_size);
    set_if(criteria, EGL_ALPHA_SIZE, attrs.alpha_size);
    set_if(criteria, EGL_DEPTH_SIZE, attrs.depth_size);
    set_if(criteria, EGL_STENCIL_SIZE, attrs.stencil_size);
    set_if(criteria, EGL_CONFIG_ID, attrs.configid);

    // it's possible to create Big GL rendering context through EGL.
    // this then requires the use of eglBindAPI to select the correct API
    // to be used by the thread.
    // However our config attributes do not provide means for saying
    // what kind of rendering context support we want from our config (ES/GL/VG)
    // so we're going to assume here that EGL is only used for GLES1/2/3
#ifdef EGL_OPENGL_ES3_BIT
    // fixme: OpenVR sdk doesn't support GLES1 configs
    // We really need to support context type bit flag properly. 
    // see issue #7
    set_if(criteria, EGL_RENDERABLE_TYPE,
       /* EGL_OPENGL_ES_BIT | */ EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT);
#else
    set_if(criteria, EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES_BIT |  EGL_OPENGL_ES2_BIT);
#endif

    // EGL 1.4 supports two buffering models. Back buffered and single buffered.
    // back buffered rendering is used by window and pbuffer surfaces automatically.
    // there's no support for tripple buffering.

    int drawable_bits = 0;
    if (attrs.surfaces.window)
        drawable_bits |= EGL_WINDOW_BIT;
    if (attrs.surfaces.pbuffer)
        drawable_bits |= EGL_PBUFFER_BIT;
    if (attrs.surfaces.pixmap)
        drawable_bits |= EGL_PIXMAP_BIT;

    set_if(criteria, EGL_SURFACE_TYPE, drawable_bits);

    if (attrs.sampling != Multisampling::None)
    {
        set_if(criteria, EGL_SAMPLE_BUFFERS, 1);
        if (attrs.sampling == Multisampling::MSAA4)
            set_if(criteria, EGL_SAMPLES, 4);
        else if (attrs.sampling == Multisampling::MSAA8)
            set_if(criteria, EGL_SAMPLES, 8);
        else if (attrs.sampling == Multisampling::MSAA16)
            set_if(criteria, EGL_SAMPLES, 16);
    }

    criteria.push_back(EGL_NONE);

    EGLint num_matches = 0;
    EGLConfig config   = NULL;
    if (!eglChooseConfig(pimpl_->display, (const EGLint*)&criteria[0], &config, 1, &num_matches))
        throw std::runtime_error("eglChooseConfig failed");
    if (!num_matches || !config)
        throw std::runtime_error("no matching framebuffer configuration available");

    pimpl_->config   = config;
    pimpl_->visualid = 0;
    pimpl_->configid = 0;
    pimpl_->srgb     = attrs.srgb_buffer;

    // Note that the visual id may or may not be zero. On windows there's no single
    // integer value that maps to a visual id, but the "visualid" is more like PIXELFORMATDESCRIPTOR.
    // So therefore at least on Imgtech GLES implementation the visualid is simply 0
    // and the driver does the pixelformat matching in the call to eglCreateWindowSurface
    eglGetConfigAttrib(pimpl_->display, config, EGL_NATIVE_VISUAL_ID, (EGLint*)&pimpl_->visualid);
    eglGetConfigAttrib(pimpl_->display, config, EGL_CONFIG_ID, (EGLint*)&pimpl_->configid);
}

Config::~Config()
{
}

uint_t Config::GetVisualID() const
{
    return pimpl_->visualid;
}

uint_t Config::GetConfigID() const
{
    return pimpl_->configid;
}

gl_config_t Config::GetNativeHandle() const
{
    return { pimpl_->config };
}

bool Config::sRGB() const
{
    return pimpl_->srgb;
}

} // wdk
