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
#include "window_listener.h"
#include "window_events.h"
#include "window.h"

namespace wdk
{

void ConnectWindowListener(wdk::Window& window, wdk::WindowListener& listener)
{
    namespace args = std::placeholders;

#ifdef WDK_MULTIPLE_WINDOW_LISTENERS
    window.on_create.listen(std::bind(&window_listener::on_create, &listener, args::_1));
    window.on_paint.listen(std::bind(&window_listener::on_paint, &listener, args::_1));
    window.on_resize.listen(std::bind(&window_listener::on_resize, &listener, args::_1));
    window.on_lost_focus.listen(std::bind(&window_listener::on_lost_focus, &listener, args::_1));
    window.on_gain_focus.listen(std::bind(&window_listener::on_gain_focus, &listener, args::_1));
    window.on_want_close.listen(std::bind(&window_listener::on_want_close, &listener, args::_1));
    window.on_keydown.listen(std::bind(&window_listener::on_keydown, &listener, args::_1));
    window.on_keyup.listen(std::bind(&window_listener::on_keyup, &listener, args::_1));
    window.on_char.listen(std::bind(&window_listener::on_char, &listener, args::_1));
    window.on_mouse_move.listen(std::bind(&window_listener::on_mouse_move, &listener, args::_1));
    window.on_mouse_press.listen(std::bind(&window_listener::on_mouse_press, &listener, args::_1));
    window.on_mouse_release.listen(std::bind(&window_listener::on_mouse_release, &listener, args::_1));
#else
    window.on_create        = std::bind(&WindowListener::OnCreate, &listener, args::_1);
    window.on_paint         = std::bind(&WindowListener::OnPaint, &listener, args::_1);
    window.on_resize        = std::bind(&WindowListener::OnResize, &listener, args::_1);
    window.on_lost_focus    = std::bind(&WindowListener::OnLostFocus, &listener, args::_1);
    window.on_gain_focus    = std::bind(&WindowListener::OnGainFocus, &listener, args::_1);
    window.on_want_close    = std::bind(&WindowListener::OnWantClose, &listener, args::_1);
    window.on_keydown       = std::bind(&WindowListener::OnKeyDown, &listener, args::_1);
    window.on_keyup         = std::bind(&WindowListener::OnKeyUp, &listener, args::_1);
    window.on_char          = std::bind(&WindowListener::OnCharacter, &listener, args::_1);
    window.on_mouse_move    = std::bind(&WindowListener::OnMouseMove, &listener, args::_1);
    window.on_mouse_press   = std::bind(&WindowListener::OnMousePress, &listener, args::_1);
    window.on_mouse_release = std::bind(&WindowListener::OnMouseRelease, &listener, args::_1);
#endif

}


} // wdk

