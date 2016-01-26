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

// wayland implementation for wdk::window

#ifndef WDK_WAYLAND
#  define WDK_WAYLAND
#endif

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <stdexcept>
#include <cassert>
#include "../window.h"
#include "../system.h"
#include "framebuffer.h"

namespace wdk
{

// double buffering. we can render into 1st buffer
// while the compositor is using the 2nd buffer and 
// once an update is ready, we well the compositor 
// to start using the other buffer etc.
using framebuff= framebuffer<2> ;

struct window::impl {
    wl_surface*       surface;
    wl_shell_surface* shell;
    wl_region*        region;
    wl_egl_window*    window;

    bool fullscreen;
    bool canresize;

    std::unique_ptr<framebuff> fb;

    uint_t width;
    uint_t height;

    // ping callback. the server pings us to check that we're 
    // receiving events and sending requests. 
    // we need to reply with a pong.
    static void shell_surface_ping(void* data, 
        wl_shell_surface* surface, uint32_t serial)
    {
        auto* self = static_cast<impl*>(data);
        wl_shell_surface_pong(self->shell, serial);
    }

    // the server is suggesting a resize of the surface.
    // the size is a hint for the new size, but we're
    // free to ignore it if we don't want to resize.
    static void shell_surface_configure(void* data,
        wl_shell_surface* surface, 
        uint32_t edges, int32_t width, int32_t height)
    {
        //auto* self = static_cast<impl*>(data);

    }

    // todo: what's the purpose of this!?
    static void shell_surface_popup_done(void* data,
        wl_shell_surface* surface)
    {

    }
};



window::window() : pimpl_(new impl)
{
    pimpl_->window     = nullptr;
    pimpl_->surface    = nullptr;
    pimpl_->shell      = nullptr;
    pimpl_->region     = nullptr;
    pimpl_->fullscreen = false;
}

window::~window()
{
    if (exists())
        destroy();
}

void window::create(const std::string& title, uint_t width, uint_t height, uint_t visualid,
    bool can_resize, bool has_border, bool initially_visible)
{
    assert(width  && "Window width cannot be zero.");
    assert(height && "Window height cannot be zero.");
    assert(pimpl_->window == nullptr &&
        "Window exists already.");

    //std::unique_ptr<framebuff> fb(new framebuff);

    auto display = get_display_handle();

    // todo: RAII

    // A surface is a rectangular area that is displayed on the screen.
    // It has a location, size and pixel contents.
    // Each surface needs to be set a "role" so that compositor 
    // knows how to deal with it. A role can be for example
    // cursor, a drag icon, a sub-surface or a window.
    wl_surface* surface = wl_compositor_create_surface(display.compositor);
    if (surface == nullptr)
        throw std::runtime_error("create wayland surface failed");

    //fb->prepare(surface, width, height);

    // set shell window role + retrieve shell interface object.
    wl_shell_surface* shell = wl_shell_get_shell_surface(display.shell, surface);
    if (shell == nullptr)
        throw std::runtime_error("create wayland shell surface failed");

    static const wl_shell_surface_listener listener = {
        &impl::shell_surface_ping,
        &impl::shell_surface_configure,
        &impl::shell_surface_popup_done
    };

    wl_shell_surface_add_listener(shell, &listener, pimpl_.get());
    wl_shell_surface_set_toplevel(shell);
    wl_shell_surface_set_title(shell, "keke");


    pimpl_->surface = surface;
    pimpl_->shell   = shell;
    pimpl_->fullscreen = false;
    pimpl_->canresize  = can_resize;
    //pimpl_->fb  = std::move(fb);
    pimpl_->width  = width;
    pimpl_->height = height;

    //auto* front = pimpl_->fb->get_current();
    //front->clear();
    //pimpl_->fb->flip();
}

void window::hide()
{

}

void window::show()
{

}

void window::destroy()
{
    assert(exists() && "Window wasn't created yet.");

    wl_shell_surface_destroy(pimpl_->shell);    
    wl_surface_destroy(pimpl_->surface);

    pimpl_->surface = nullptr;
    pimpl_->shell   = nullptr;
    pimpl_->region  = nullptr;
    pimpl_->window  = nullptr;
    pimpl_->fullscreen = false;

}

void window::move(int x, int y)
{

}

void window::set_fullscreen(bool fullscreen)
{

}

void window::set_focus()
{}

void window::set_size(uint_t width, uint_t height)
{}

void window::set_encoding(encoding enc)
{}

void window::poll_one_event()
{}

void window::wait_one_event()
{
    auto display = wdk::get_display_handle();

    wl_display_dispatch(display.display);
}

void window::process_all_events()
{}

void window::process_event(const native_event_t& ev)
{}

void window::sync_all_events()
{}

uint_t window::surface_width() const 
{
    return pimpl_->height;
}

uint_t window::surface_height() const
{
    return pimpl_->width;
} 

bool window::exists() const 
{
    return pimpl_->surface != nullptr;
}

bool window::is_fullscreen() const 
{
    return pimpl_->fullscreen;
}

window::encoding window::get_encoding() const 
{
    return encoding::ascii;
}

native_window_t window::handle() const 
{
    return pimpl_->surface;
}

egl_handle_t  window::egl_handle() const 
{
    if (pimpl_->window)
        return pimpl_->window;

    auto disp = get_display_handle();

    pimpl_->region = wl_compositor_create_region(disp.compositor);
    if (pimpl_->region == nullptr)
        throw std::runtime_error("create wayland region failed");

    wl_region_add(pimpl_->region, 0, 0, 
        pimpl_->width, pimpl_->height);
    wl_surface_set_opaque_region(pimpl_->surface, pimpl_->region);

    pimpl_->window = wl_egl_window_create(pimpl_->surface,
        pimpl_->width, pimpl_->height);
    if (pimpl_->window == nullptr)
        throw std::runtime_error("create egl window failed");

    return pimpl_->window;
}

} // wdk
