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

#include <EGL/egl.h>
#include <stdexcept>
#include <vector>
#include <cassert>
#include "../system.h"
#include "../config.h"
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

config::attributes config::DONT_CARE = {0, 0, 0, 0, 0, 0, 0, 0, true, {true, false, false}, multisampling::none};
config::attributes config::DEFAULT = {8, 8, 8, 8, 8, 8, 0, 0, true, {true, false, false}, multisampling::none};

struct config::impl {
    EGLDisplay   display;
    EGLConfig    config;
    uint_t       visualid;
    uint_t       configid;
};

config::config(const attributes& attrs) : pimpl_(new impl)
{
    pimpl_->display = egl_init(get_display_handle());

    std::vector<uint_t> criteria;

    set_if(criteria, EGL_RED_SIZE, attrs.red_size);
    set_if(criteria, EGL_GREEN_SIZE, attrs.green_size);
    set_if(criteria, EGL_BLUE_SIZE, attrs.blue_size);
    set_if(criteria, EGL_ALPHA_SIZE, attrs.alpha_size);
    set_if(criteria, EGL_DEPTH_SIZE, attrs.depth_size);
    set_if(criteria, EGL_STENCIL_SIZE, attrs.stencil_size);    
    set_if(criteria, EGL_CONFIG_ID, attrs.configid);
    set_if(criteria, EGL_NATIVE_VISUAL_ID, attrs.visualid);

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

    if (attrs.sampling != multisampling::none)
    {
        set_if(criteria, EGL_SAMPLE_BUFFERS, 1);
        if (attrs.sampling == multisampling::msaa4)
            set_if(criteria, EGL_SAMPLES, 4);
        else if (attrs.sampling == multisampling::msaa8)
            set_if(criteria, EGL_SAMPLES, 8);
        else if (attrs.sampling == multisampling::msaa16)
            set_if(criteria, EGL_SAMPLES, 16);
    }

    criteria.push_back(EGL_NONE);

    EGLint num_matches = 0;
    EGLConfig config   = NULL;
    if (!eglChooseConfig(pimpl_->display, (const EGLint*)&criteria[0], &config, 1, &num_matches))
        throw std::runtime_error("no matching framebuffer configuration available");

    pimpl_->config   = config;
    pimpl_->visualid = 0;
    pimpl_->configid = 0;

    eglGetConfigAttrib(pimpl_->display, config, EGL_NATIVE_VISUAL_ID, (EGLint*)&pimpl_->visualid);
    eglGetConfigAttrib(pimpl_->display, config, EGL_CONFIG_ID, (EGLint*)&pimpl_->configid);

    // should there always be visualid? one would think that this is the case even on windows 
    // (pixelformatdescriptor??) but at least with imgtech gles2/2 there's no visualid! 
    // assert(pimpl_->visualid);
    // assert(pimpl_->configid);
}

config::~config()
{
}

uint_t config::visualid() const
{
    return pimpl_->visualid;
}

uint_t config::configid() const
{
    return pimpl_->configid;
}

gl_config_t config::handle() const
{
    return { pimpl_->config };
}


} // wdk
