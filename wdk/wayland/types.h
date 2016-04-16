// Copyright (c) 2010-2016 Sami Väisänen, Ensisoft 
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

// window system types specific to Wayland.

#include <typeinfo>
#include <memory>
#include <cstdlib> // for abort

struct wl_display;
struct wl_compositor;
struct wl_shell;
struct wl_surface;
struct wl_egl_window;
struct wl_shm;

namespace wdk
{
    struct native_display_t {
        wl_display*    display;
        wl_compositor* compositor;
        wl_shell*      shell;
        wl_shm*        shm;
    };

    typedef wl_surface*    native_window_t;
    typedef wl_surface*    native_pixmap_t;
    typedef void*          native_drawable_t;    
    typedef wl_egl_window* egl_handle_t;
    typedef wl_display*    egl_display_t;

    class native_event_t
    {
    public:
        enum class type {
            none,

            window_gain_focus,
            window_lost_focus,
            window_resize,
            window_create,
            window_destroy,
            window_keydown,
            window_keyup,
            window_char,
            window_paint,
            window_mouse_move,
            window_mouse_press,
            window_mouse_release
         };

        struct paint_region {
            int x, y;
            int width, height;
        };

        native_event_t() : m_type(type::none), m_window(nullptr)
        {}

        template<typename T>
        native_event_t(const T& data, type t, wl_surface* win) : m_type(t),  
            m_window(win)
        {
            m_event.reset(new event_holder<T>(data));
        }

        template<typename T>
        T& as() 
        {
            void* p = m_event->test(typeid(T));
            if (p == nullptr)
                std::abort();
            return *static_cast<T*>(p);
        }

        template<typename T>
        const T& as() const 
        {
            const void* p = m_event->test(typeid(T));
            if (p == nullptr)
                std::abort();
            return *static_cast<const T*>(p);
        }

        template<typename T>
        T* query()
        {
            void* p = m_event->test(typeid(T));
            return static_cast<T*>(p);
        }

        template<typename T>
        const T* query() const 
        {
            const void* p = m_event->test(typeid(T));
            return static_cast<const T*>(p);
        }

        native_window_t get_window_handle() const 
        { return m_window; }

        type identity() const 
        { return m_type; }

    private:
        class event 
        {
        public:
            virtual ~event() = default;
            virtual void* test(const std::type_info& mebbe) = 0;
        private:
        protected:
        };

        template<typename T>
        class event_holder : public event
        {
        public:
            event_holder(const T data) : m_data(data)
            {}
            virtual void* test(const std::type_info& mebbe) override
            {
                if (mebbe != typeid(T))
                    return nullptr;
                return &m_data;
            }
        private:
            T m_data;
        };
        std::shared_ptr<event> m_event;
    private:
        type m_type;
    private:
        wl_surface* m_window;
    };

} // wdk
