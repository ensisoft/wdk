// Copyright (c) 2013-2016 Sami Väisänen, Ensisoft 
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

// for clang in SublimeText 2
#ifndef WDK_WAYLAND
#  define WDK_WAYLAND
#endif

#pragma once

#include <wayland-client.h>
#include <wayland-server.h>
#include <wdk/utility.h>
#include <wdk/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <memory>
#include <cassert>
#include <cstring>
#include "../system.h"

namespace wdk
{
    // linear memory framebuffer.
    template<std::size_t N>
    class framebuffer : noncopyable
    {
    public:
        class shared_mem_buffer : noncopyable
        {
        public:
            using u8 = std::uint8_t;

            u8& operator[](std::size_t i)
            {
                auto* p = (u8*)m_base;
                assert(i < m_size);
                return p[i];
            }

            const u8& operator[](std::size_t i) const 
            { 
                auto* p = (u8*)m_base;
                assert(i < m_size);
                return p[i];
            }

            void clear()
            {
                std::memset(m_base, 0xff, m_size);
            }

            void* ptr()
            { return m_base; }

            const void* ptr() const 
            { return m_base; }
        private:
            template <std::size_t > friend class framebuffer;

            shared_mem_buffer() : m_buff(nullptr)
            {}

           ~shared_mem_buffer()
            {
                if (m_buff)
                    wl_buffer_destroy(m_buff);
            }

            void create(void* base, wl_buffer* buff, size_t size)
            {
                m_base = base;
                m_buff = buff;
                m_size = size;
            }
        private:
            size_t m_size;
            void* m_base;
            wl_buffer* m_buff;
        };

        framebuffer()
        {}

        framebuffer(wl_surface* target, std::size_t width, std::size_t height)
        {
            prepare(target, width, height);
        }

        void prepare(wl_surface* target, std::size_t width, std::size_t height)
        {
            assert(m_pool == nullptr);

            const auto disp  = get_display_handle();
            const auto page  = width * height * 4;            
            const auto total = page * N;

            m_shmem.reset(new shared_memory);
            m_shmem->create("/foo", total);

            m_pool = wl_shm_create_pool(disp.shm, m_shmem->fd(), total);
            if (!m_pool) 
                throw std::runtime_error("failed to create shm pool");

            m_shmem->unlink();

            auto* p = (std::uint8_t*)m_shmem->mem();

    //#ifndef _NDEBUG
            std::memset(p, 0xff, total);
    //#endif

            for (std::size_t i=0; i<N; ++i)
            {
                const auto offset = i * page;
                const auto stride = width * 4; // bytes
                wl_buffer* buff = wl_shm_pool_create_buffer(
                    m_pool, offset, width, height, 
                    stride, WL_SHM_FORMAT_ARGB8888);
                if (buff == nullptr)
                    throw std::runtime_error("failed to create wl_buffer");

                m_buffers[i].create(p + offset, buff, page);
            }

            m_buff_index = 0;
            m_surface    = target;
            m_width      = width;
            m_height     = height;
        }

       ~framebuffer()
        {
            wl_shm_pool_destroy(m_pool);
        }

        shared_mem_buffer* get_current() 
        {
            return &m_buffers[m_buff_index];
        }

        shared_mem_buffer* flip()
        {
            assert(m_surface != nullptr);

            shared_mem_buffer* current = get_current();
            wl_surface_attach(m_surface, current->m_buff,
                0, 0);
            wl_surface_damage(m_surface,
                0, 0, m_width, m_height);
            wl_surface_commit(m_surface);
            m_buff_index = (m_buff_index + 1) % N;
            return &m_buffers[m_buff_index];
        }

    private:
        class shared_memory : noncopyable
        {
        public:
            shared_memory() : m_size(0), m_base(nullptr), m_fd(0)
            {}

           ~shared_memory()
            {
                if (m_base)
                    ::munmap(m_base, m_size);
                if (m_fd)
                    ::close(m_fd);
                if (!m_name.empty())
                    ::shm_unlink(m_name.c_str());

            }
            void create(const std::string& name, std::size_t size)
            {
                assert(m_base == nullptr);

                m_fd = ::shm_open(name.c_str(), O_RDWR | O_CREAT, 0);
                if (m_fd == -1)
                    throw std::runtime_error("failed to create shared memory");

                if (::ftruncate(m_fd, size) == -1)
                    throw std::runtime_error("failed to resize shared memory");
                m_base = ::mmap(nullptr, size, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE, m_fd, 0);
                if (m_base == nullptr)
                    throw std::runtime_error("memory mapping failed");

                m_size = size;
                m_name = name;
            }

            void* mem()
            { return m_base; }

            int fd() 
            { return m_fd; }

            void unlink() 
            {
                ::close(m_fd); 
                ::shm_unlink(m_name.c_str()); 

                m_fd   = 0;
                m_name = "";
            }

        private:
            std::string m_name;
            std::size_t m_size;
            void* m_base;
        private:
            int m_fd;
        };

    private:
        std::unique_ptr<shared_memory> m_shmem;
        std::size_t m_width;
        std::size_t m_height;
    private:
        std::size_t m_buff_index;        
        shared_mem_buffer m_buffers[N];
    private:
        wl_shm_pool* m_pool;
        wl_surface*  m_surface;
    };


} // wdk
