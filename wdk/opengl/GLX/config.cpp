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

#include <GL/glx.h>
#include <stdexcept>
#include <vector>
#include <wdk/system.h>
#include <wdk/utility.h>
#include "../config.h"

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

config::attributes config::DONT_CARE = {0, 0, 0, 0, 0, 0, 0, 0, false, {true, false, false}, multisampling::none};
config::attributes config::DEFAULT = {8, 8, 8, 8, 16, 8, 0, 0, true, {true, false, false}, multisampling::none};

struct config::impl {
    GLXFBConfig* configs;
    GLXFBConfig  config;
    uint_t       visualid;
    uint_t       configid;
};

config::config(const attributes& attrs) : pimpl_(new impl)
{
    std::vector<uint_t> criteria = 
    {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_X_RENDERABLE, True,
    };
        
    set_if(criteria, GLX_RED_SIZE,     attrs.red_size);
    set_if(criteria, GLX_GREEN_SIZE,   attrs.green_size);
    set_if(criteria, GLX_BLUE_SIZE,    attrs.blue_size);
    set_if(criteria, GLX_ALPHA_SIZE,   attrs.alpha_size);
    set_if(criteria, GLX_DEPTH_SIZE,   attrs.depth_size);
    set_if(criteria, GLX_STENCIL_SIZE, attrs.stencil_size);    
    set_if(criteria, GLX_FBCONFIG_ID,  attrs.configid);
    set_if(criteria, GLX_VISUAL_ID,    attrs.visualid);

    int drawable_bits = 0;
    if (attrs.surfaces.window)
        drawable_bits |= GLX_WINDOW_BIT;
    if (attrs.surfaces.pbuffer)
        drawable_bits |= GLX_PBUFFER_BIT;
    if (attrs.surfaces.pixmap)
        drawable_bits |= GLX_PIXMAP_BIT;

    set_if(criteria, GLX_DRAWABLE_TYPE, drawable_bits);

    if (attrs.double_buffer)
        set_if(criteria, GLX_DOUBLEBUFFER, (uint_t)True);

    if (attrs.sampling != multisampling::none)
    {
        set_if(criteria, GLX_SAMPLE_BUFFERS, 1);
        if (attrs.sampling == multisampling::msaa4)
            set_if(criteria, GLX_SAMPLES, 4);
        else if (attrs.sampling == multisampling::msaa8)
            set_if(criteria, GLX_SAMPLES, 8);
        else if (attrs.sampling == multisampling::msaa16)
            set_if(criteria, GLX_SAMPLES, 16);
    }

    criteria.push_back(None);

    auto dpy = get_display_handle();

    int num_matches = 0;
    auto matches = make_unique_ptr(glXChooseFBConfig(dpy, DefaultScreen(dpy), (const int*)&criteria[0], &num_matches), XFree);
    if (!matches.get() || !num_matches)
        throw std::runtime_error("no matching framebuffer configuration available");

    GLXFBConfig* configs = matches.get();

    int best_index = 0;

    // choose a configuration from the list of matching configurations.
    // todo: sort matches?


    // hmmm.. it seems that the GLX_VISUAL_ID option is not really considered properly
    // so we check here. 
    if (attrs.visualid)
    {
        bool found = false;
        for (; best_index < num_matches; ++best_index)
        {
            const auto conf = configs[best_index];
            const auto visu = make_unique_ptr(glXGetVisualFromFBConfig(dpy, conf), XFree);
            if (attrs.visualid == visu->visualid)
            {
                found = true;
                break;
            }
        }
        if (!found)
            throw std::runtime_error("no matching framebuffer configuration available");
    }

    GLXFBConfig best = configs[best_index];

    auto visual = make_unique_ptr(glXGetVisualFromFBConfig(dpy, best), XFree);

    pimpl_->configs  = matches.release();
    pimpl_->config   = best;
    pimpl_->visualid = visual->visualid;
    pimpl_->configid = 0;

    glXGetFBConfigAttrib(dpy, best, GLX_FBCONFIG_ID, (int*)&pimpl_->configid);
}

config::~config()
{
    XFree(pimpl_->configs);
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
    return {pimpl_->config};
}

} // wdk
