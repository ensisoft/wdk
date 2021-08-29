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

#include <functional>

#include "wdk/listener.h"
#include "wdk/events.h"
#include "wdk/window.h"

namespace wdk
{

void Connect(wdk::Window& window, wdk::WindowListener& listener)
{
    namespace args = std::placeholders;

#ifdef WDK_MULTIPLE_WINDOW_LISTENERS
    window.on_create.Bind(std::bind(&WindowListener::OnCreate, &listener, args::_1));
    window.on_paint.Bind(std::bind(&WindowListener::OnPaint, &listener, args::_1));
    window.on_resize.Bind(std::bind(&WindowListener::OnResize, &listener, args::_1));
    window.on_lost_focus.Bind(std::bind(&WindowListener::OnLostFocus, &listener, args::_1));
    window.on_gain_focus.Bind(std::bind(&WindowListener::OnGainFocus, &listener, args::_1));
    window.on_want_close.Bind(std::bind(&WindowListener::OnWantClose, &listener, args::_1));
    window.on_keydown.Bind(std::bind(&WindowListener::OnKeydown, &listener, args::_1));
    window.on_keyup.Bind(std::bind(&WindowListener::OnKeyup, &listener, args::_1));
    window.on_char.Bind(std::bind(&WindowListener::OnChar, &listener, args::_1));
    window.on_mouse_move.Bind(std::bind(&WindowListener::OnMouseMove, &listener, args::_1));
    window.on_mouse_press.Bind(std::bind(&WindowListener::OnMousePress, &listener, args::_1));
    window.on_mouse_release.Bind(std::bind(&WindowListener::OnMouseRelease, &listener, args::_1));
#else
    window.on_create        = std::bind(&WindowListener::OnCreate, &listener, args::_1);
    window.on_paint         = std::bind(&WindowListener::OnPaint, &listener, args::_1);
    window.on_resize        = std::bind(&WindowListener::OnResize, &listener, args::_1);
    window.on_lost_focus    = std::bind(&WindowListener::OnLostFocus, &listener, args::_1);
    window.on_gain_focus    = std::bind(&WindowListener::OnGainFocus, &listener, args::_1);
    window.on_want_close    = std::bind(&WindowListener::OnWantClose, &listener, args::_1);
    window.on_keydown       = std::bind(&WindowListener::OnKeydown, &listener, args::_1);
    window.on_keyup         = std::bind(&WindowListener::OnKeyup, &listener, args::_1);
    window.on_char          = std::bind(&WindowListener::OnChar, &listener, args::_1);
    window.on_mouse_move    = std::bind(&WindowListener::OnMouseMove, &listener, args::_1);
    window.on_mouse_press   = std::bind(&WindowListener::OnMousePress, &listener, args::_1);
    window.on_mouse_release = std::bind(&WindowListener::OnMouseRelease, &listener, args::_1);
#endif

}


} // wdk

