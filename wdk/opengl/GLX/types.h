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

#pragma once

// do not include GL/glx.h in this header because glx.h includes gl.h
// which defines the GL types such as GLint and on some systems
// when client code includes GLES2/gl[2].h the type definitions
// are conflicting (GLintptr, GLsizeiptr). So instead of polluting
// the client namespace (wdk doesn't need or care about GL types directly)
// the inclusion is removed.
//#include <GL/glx.h>

// actually we'll want to remove this pollution as well.
//#include <X11/Xlib.h>

// GLX specific types.

struct __GLXFBConfigRec;
struct __GLXcontextRec;

typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef struct __GLXcontextRec *GLXContext;

//typedef XID GLXDrawable;

namespace wdk
{
    namespace detail {

        typedef unsigned long XID;
        typedef XID GLXDrawable;

    } // detail

    template<typename T, int discriminator>
    struct glx_t {
        T glx;

        operator T () const
        {
            return glx;
        }
    };

    template<typename T, int discriminator> inline
    bool operator==(const glx_t<T, discriminator>& rhs, const glx_t<T, discriminator>& lhs)
    {
        return rhs.xid == lhs.xid;
    }

    template<typename T, int discriminator> inline
    bool operator!=(const glx_t<T, discriminator>& rhs, const glx_t<T, discriminator>& lhs)
    {
        return rhs.xid != lhs.xid;
    }

    typedef glx_t<detail::GLXDrawable, 0> gl_surface_t;
    typedef glx_t<GLXFBConfig, 1> gl_config_t;

} // wdk
