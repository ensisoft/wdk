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
#include "../config.h"
#include "../system.h"
#include "../utility.h"

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

config::attributes config::DONT_CARE = {0, 0, 0, 0, 0, 0, 0, boost::indeterminate, {true, false, false}};
config::attributes config::DEFAULT = {8, 8, 8, 8, 16, 0, 0, true, {true, false, false}};

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

    if (!dont_care(attrs.doublebuffer) && attrs.doublebuffer)
        set_if(criteria, GLX_DOUBLEBUFFER, (uint_t)True);

    criteria.push_back(None);

    Display* dpy = get_display_handle();

    int num_matches = 0;
    auto matches = make_unique_ptr(glXChooseFBConfig(dpy, DefaultScreen(dpy), (const int*)&criteria[0], &num_matches), XFree);
    if (!matches.get() || !num_matches)
        throw std::runtime_error("no matching framebuffer configuration available");

    // choose a configuration from the list of matching configurations.
    // todo: sort matches?

    GLXFBConfig best = matches.get()[0];

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
    return gl_config_t {pimpl_->config};
}

} // wdk
