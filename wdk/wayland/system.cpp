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

#include <sys/select.h>
#include <wayland-client.h>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <queue>
#include "../system.h"
#include "../videomode.h"

#define UNIMPLEMENTED throw std::runtime_error("unimplemented function");

namespace wdk
{

std::queue<native_event_t> g_queue;

native_display_t get_display_handle()
{
    struct display {
        wl_display* d;
        wl_compositor* c;
        wl_shell* s;
        wl_registry* r;
        wl_shm* shm;
        wl_seat* seat;
        wl_keyboard* keyboard;
        wl_pointer* mouse;

        wl_surface* input_focus_surface;

        display() : d(wl_display_connect(nullptr))
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
            else if (shm == nullptr)
                throw std::runtime_error("failed to get wayland shared memory manager");
            else if (seat == nullptr)
                throw std::runtime_error("failed to get wayland input peripheral manager");

            keyboard = wl_seat_get_keyboard(seat);
            if (keyboard) 
            {
                const static wl_keyboard_listener list[] = {
                    keyboard_keymap,
                    keyboard_enter,
                    keyboard_leave,
                    keyboard_keypress,
                    keyboard_modifier_state,
                    keyboard_repeat_state
                };
                wl_keyboard_add_listener(keyboard, list, this);
            }

            mouse    = wl_seat_get_pointer(seat);
            input_focus_surface = nullptr;
        }
       ~display()
        {
            if (keyboard)
                wl_keyboard_destroy(keyboard);
            if (mouse)
                wl_pointer_destroy(mouse);

            wl_seat_destroy(seat);
            wl_shm_destroy(shm);
            wl_shell_destroy(s);
            wl_compositor_destroy(c);
            wl_registry_destroy(r);            
            wl_display_disconnect(d);            
        }

        static
        void keyboard_keymap(void* data, wl_keyboard* keyboard,
            uint32_t format, int32_t fd, uint32_t size)
        {}

        static
        void keyboard_enter(void* data, wl_keyboard* keyboard,
            uint32_t serial, wl_surface* surface, wl_array* keys)
        {}

        static
        void keyboard_leave(void* data, wl_keyboard* keyboard,
            uint32_t serial, wl_surface* surface)
        {
            auto* self = static_cast<display*>(data);
        }

        static 
        void keyboard_keypress(void* data, wl_keyboard* keyboard,
            uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
        {

        }



        static 
        void keyboard_modifier_state(void* data, wl_keyboard* keyboard,
            uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
            uint32_t group)
        {

        }

        static
        void keyboard_repeat_state(void* data, wl_keyboard* keyboard,
            int32_t rate, int32_t delay)
        {}

        static 
        void global_registry_handler(void* data, struct wl_registry* registry,
            uint32_t id, const char* interface, uint32_t version)
        {
            auto* self = static_cast<display*>(data);

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
            else if (!std::strcmp(interface, wl_seat_interface.name))
            {
                self->seat = (wl_seat*)wl_registry_bind(registry, id,
                    &wl_seat_interface, 1);
            }
        }

        static
        void global_registry_remover(void* data, struct wl_registry* registry,
            uint32_t id)
        {
            // todo: need to do something here?
        }
    };

    static display dpy;

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
    return !g_queue.empty();
}

bool sync_events()
{
    auto disp = get_display_handle();

    wl_display_dispatch_pending(disp.display);

    return true;
}

native_event_t get_event()
{
    auto disp = get_display_handle();

    const auto fd = wl_display_get_fd(disp.display);
    fd_set read;
    fd_set write;
    FD_ZERO(&read);
    FD_ZERO(&write);
    FD_SET(fd, &read);
    FD_SET(fd, &write);

    while (g_queue.empty())
    {
        wl_display_dispatch_pending(disp.display);
        if (!g_queue.empty())
            break;

        int ret = wl_display_flush(disp.display);        
        if (ret == 0)
            FD_ZERO(&write);
        else if (ret == -1 && errno == EAGAIN)
            FD_SET(fd, &write);

        ret = ::select(fd + 1, &read, &write, nullptr, nullptr);
        if (ret == -1)
            throw std::runtime_error("select failed");
    }

    assert(!g_queue.empty());

    auto ret = g_queue.front();
    g_queue.pop();

    return ret;
}

bool peek_event(native_event_t& ev)
{
    if (g_queue.empty())
        return false;

    ev = g_queue.front();
    return true;
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