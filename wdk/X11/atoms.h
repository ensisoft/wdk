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

#include <X11/Xlib.h>

namespace wdk
{

// see the extended window manager hints here.
// http://standards.freedesktop.org/wm-spec/wm-spec-latest.html

extern Atom _NET_WM_STATE;
extern Atom _NET_WM_STATE_FULLSCREEN;
extern Atom _MOTIF_WM_HINTS;

extern Atom WM_SIZE_HINTS;
extern Atom WM_DELETE_WINDOW;

// if these are "extern long const" they appear as undefined references *and* defined references.
// causing linker errors later. smells like an issue in the toolchain!?!
//
// bin/libwdk_systemd.a:system.cpp.o:0000000000000000 B wdk::_NET_WM_STATE
// bin/libwdk_systemd.a:window.cpp.o:                 U wdk::_NET_WM_STATE
//
extern long _NET_WM_STATE_REMOVE;
extern long _NET_WM_STATE_ADD;
extern long _NET_WM_STATE_TOGGLE;

extern int AltMask;
extern int XRandREventBase;

} // wdk
