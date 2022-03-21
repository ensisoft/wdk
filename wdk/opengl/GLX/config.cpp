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

#include "wdk/system.h"
#include "wdk/utility.h"
#include "wdk/opengl/config.h"

#define X11_None 0

// from EXT_framebuffer_sRGB.txt
// Accepted by the <attribList> parameter of glXChooseVisual, and by
// the <attrib> parameter of glXGetConfig:
#define GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT             0x20B2

namespace {
void set(std::vector<wdk::uint_t>& v, wdk::uint_t attr, wdk::uint_t value)
{
    v.push_back(attr);
    v.push_back(value);
}
void set_if(std::vector<wdk::uint_t>& v, wdk::uint_t attr, wdk::uint_t value)
{
    if (value)
    {
        v.push_back(attr);
        v.push_back(value);
    }
}
wdk::Config::Attributes GetDefaultAttrs()
{
    wdk::Config::Attributes attrs;
    attrs.red_size         = 8;
    attrs.green_size       = 8;
    attrs.blue_size        = 8;
    attrs.alpha_size       = 8;
    attrs.depth_size       = 16;
    attrs.stencil_size     = 8;
    attrs.configid         = 0;
    attrs.double_buffer    = true;
    attrs.srgb_buffer      = true;
    attrs.surfaces.window  = true;
    attrs.surfaces.pbuffer = false;
    attrs.surfaces.pixmap  = false;
    attrs.sampling         = wdk::Config::Multisampling::None;
    return attrs;
}
wdk::Config::Attributes GetDontCareAttrs()
{
    wdk::Config::Attributes attrs;
    attrs.red_size   = 0;
    attrs.green_size = 0;
    attrs.blue_size  = 0;
    attrs.alpha_size = 0;
    attrs.depth_size = 0;
    attrs.configid   = 0;
    attrs.double_buffer = wdk::TriBool::State::NotSet;
    attrs.srgb_buffer   = wdk::TriBool::State::NotSet;
    attrs.surfaces.window = true;
    attrs.surfaces.pixmap = false;
    attrs.surfaces.pixmap = false;
    attrs.sampling        = wdk::Config::Multisampling::None;
    return attrs;
}
} // namespace

namespace wdk
{

Config::Attributes Config::DONT_CARE = GetDontCareAttrs();
Config::Attributes Config::DEFAULT   = GetDefaultAttrs();

struct Config::impl {
    GLXFBConfig* configs;
    GLXFBConfig  config;
    uint_t       visualid;
    uint_t       configid;
    bool         srgb;
};

Config::Config(const Attributes& attrs) : pimpl_(new impl)
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

    int drawable_bits = 0;
    if (attrs.surfaces.window)
        drawable_bits |= GLX_WINDOW_BIT;
    if (attrs.surfaces.pbuffer)
        drawable_bits |= GLX_PBUFFER_BIT;
    if (attrs.surfaces.pixmap)
        drawable_bits |= GLX_PIXMAP_BIT;

    set_if(criteria, GLX_DRAWABLE_TYPE, drawable_bits);

    if (attrs.double_buffer.HasValue())
        set(criteria, GLX_DOUBLEBUFFER, attrs.double_buffer.Value());
    else set(criteria, GLX_DOUBLEBUFFER, GLX_DONT_CARE);

    if (attrs.srgb_buffer.HasValue())
        set(criteria, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, attrs.srgb_buffer.Value());
    else set(criteria, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, GLX_DONT_CARE);

    if (attrs.sampling != Multisampling::None)
    {
        set(criteria, GLX_SAMPLE_BUFFERS, 1);
        if (attrs.sampling == Multisampling::MSAA4)
            set(criteria, GLX_SAMPLES, 4);
        else if (attrs.sampling == Multisampling::MSAA8)
            set(criteria, GLX_SAMPLES, 8);
        else if (attrs.sampling == Multisampling::MSAA16)
            set(criteria, GLX_SAMPLES, 16);
    }

    criteria.push_back(X11_None);

    auto dpy = GetNativeDisplayHandle();

    int num_matches = 0;
    auto matches = MakeUniqueHandle(glXChooseFBConfig(dpy, DefaultScreen(dpy), (const int*)&criteria[0], &num_matches), XFree);
    if (!matches.get() || !num_matches)
        throw std::runtime_error("no matching framebuffer configuration available");

    GLXFBConfig* configs = matches.get();

    int best_index = 0;

    // choose a configuration from the list of matching configurations.
    // todo: sort matches?
    GLXFBConfig best = configs[best_index];

    int srgb_buffer = 0;
    int config_id   = 0;
    glXGetFBConfigAttrib(dpy, best, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, &srgb_buffer);
    glXGetFBConfigAttrib(dpy, best, GLX_FBCONFIG_ID, &config_id);
    auto visual = MakeUniqueHandle(glXGetVisualFromFBConfig(dpy, best), XFree);

    pimpl_->configs  = matches.release();
    pimpl_->config   = best;
    pimpl_->visualid = visual->visualid;
    pimpl_->configid = 0;
    pimpl_->srgb     = srgb_buffer;
    pimpl_->configid = config_id;
}

Config::~Config()
{
    XFree(pimpl_->configs);
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
    return {pimpl_->config};
}

bool Config::sRGB() const 
{
    return pimpl_->srgb;
}

} // wdk
