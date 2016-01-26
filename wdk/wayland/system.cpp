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

// for clang in SublimeText2. 
#ifndef WDK_WAYLAND
#  define WDK_WAYLAND
#endif


#include <wayland-client.h>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include "../system.h"
#include "../videomode.h"

#define UNIMPLEMENTED throw std::runtime_error("unimplemented function");

namespace wdk
{

native_display_t get_display_handle()
{
    struct open_display {
        wl_display* d;
        wl_compositor* c;
        wl_shell* s;
        wl_registry* r;
        wl_shm* shm;

        open_display() : d(wl_display_connect(nullptr))
        {
            if (!d)
                throw std::runtime_error("failed to connect to wayland display");

            r = wl_display_get_registry(d);
            if (r == nullptr)
                throw std::runtime_error("wl_display_get_registry failed");

            static const wl_registry_listener callbacks[] = {
                global_registry_handler,
                global_registry_remover
            };

            wl_registry_add_listener(r, callbacks, this);
            wl_display_dispatch(d);
            wl_display_roundtrip(d);

            if (c == nullptr)
                throw std::runtime_error("failed to get wayland compositor");
            else if (s == nullptr)
                throw std::runtime_error("failed to get wayland shell");

        }
       ~open_display()
        {
            wl_shm_destroy(shm);
            wl_shell_destroy(s);
            wl_compositor_destroy(c);
            wl_registry_destroy(r);            
            wl_display_disconnect(d);            
        }

        static 
        void global_registry_handler(void* data, struct wl_registry* registry,
            uint32_t id, const char* interface, uint32_t version)
        {
            auto* self = static_cast<open_display*>(data);

            if (!std::strcmp(interface, wl_compositor_interface.name))
            {
                self->c = (wl_compositor*)wl_registry_bind(registry, id, 
                    &wl_compositor_interface, 1);
            }
            else if (!std::strcmp(interface, wl_shell_interface.name))
            {
                self->s = (wl_shell*)wl_registry_bind(registry, id,
                    &wl_shell_interface, 1);
            }
            else if (!std::strcmp(interface, wl_shm_interface.name))
            {
                self->shm = (wl_shm*)wl_registry_bind(registry, id,
                    &wl_shm_interface, 1);
            }
        }

        static
        void global_registry_remover(void* data, struct wl_registry* registry,
            uint32_t id)
        {
            // todo: need to do something here?
        }
    };

    static open_display dpy;

    return { dpy.d, dpy.c, dpy.s, dpy.shm };
}

egl_display_t get_display_handle_egl()
{
    return get_display_handle().display;
}

videomode get_current_video_mode()
{
    UNIMPLEMENTED
}

void set_video_mode(const videomode& vm)
{
    UNIMPLEMENTED
}

std::vector<videomode> list_video_modes()
{
    UNIMPLEMENTED
}

bool have_events()
{
    UNIMPLEMENTED
}

bool sync_events()
{
    UNIMPLEMENTED
}

native_event_t get_event()
{
    UNIMPLEMENTED
}

bool peek_event(native_event_t& ev)
{
    UNIMPLEMENTED
}

std::pair<bitflag<keymod>, keysym> translate_keydown_event(const native_event_t& key)
{
    UNIMPLEMENTED
}

std::pair<bitflag<keymod>, button> translate_mouse_button_event(const native_event_t& btn)
{
    UNIMPLEMENTED
}

bool test_key_down(keysym symbol)
{
    UNIMPLEMENTED
}

bool test_key_down(uint_t keycode)
{
    UNIMPLEMENTED
}

uint_t keysym_to_keycode(keysym symbol)
{
    UNIMPLEMENTED
}



} // wdk