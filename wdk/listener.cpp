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
    window.OnCreate.Bind(std::bind(&WindowListener::OnCreate, &listener, args::_1));
    window.OnPaint.Bind(std::bind(&WindowListener::OnPaint, &listener, args::_1));
    window.OnResize.Bind(std::bind(&WindowListener::OnResize, &listener, args::_1));
    window.OnLostFocus.Bind(std::bind(&WindowListener::OnLostFocus, &listener, args::_1));
    window.OnGainFocus.Bind(std::bind(&WindowListener::OnGainFocus, &listener, args::_1));
    window.OnWantClose.Bind(std::bind(&WindowListener::OnWantClose, &listener, args::_1));
    window.OnKeyDown.Bind(std::bind(&WindowListener::OnKeydown, &listener, args::_1));
    window.OnKeyUp.Bind(std::bind(&WindowListener::OnKeyup, &listener, args::_1));
    window.OnChar.Bind(std::bind(&WindowListener::OnChar, &listener, args::_1));
    window.OnMouseMove.Bind(std::bind(&WindowListener::OnMouseMove, &listener, args::_1));
    window.OnMousePress.Bind(std::bind(&WindowListener::OnMousePress, &listener, args::_1));
    window.OnMouseRelease.Bind(std::bind(&WindowListener::OnMouseRelease, &listener, args::_1));
#else
    window.OnCreate       = std::bind(&WindowListener::OnCreate, &listener, args::_1);
    window.OnPaint        = std::bind(&WindowListener::OnPaint, &listener, args::_1);
    window.OnResize       = std::bind(&WindowListener::OnResize, &listener, args::_1);
    window.OnLostFocus    = std::bind(&WindowListener::OnLostFocus, &listener, args::_1);
    window.OnGainFocus    = std::bind(&WindowListener::OnGainFocus, &listener, args::_1);
    window.OnWantClose    = std::bind(&WindowListener::OnWantClose, &listener, args::_1);
    window.OnKeyDown      = std::bind(&WindowListener::OnKeyDown, &listener, args::_1);
    window.OnKeyUp        = std::bind(&WindowListener::OnKeyUp, &listener, args::_1);
    window.OnChar         = std::bind(&WindowListener::OnChar, &listener, args::_1);
    window.OnMouseMove    = std::bind(&WindowListener::OnMouseMove, &listener, args::_1);
    window.OnMousePress   = std::bind(&WindowListener::OnMousePress, &listener, args::_1);
    window.OnMouseRelease = std::bind(&WindowListener::OnMouseRelease, &listener, args::_1);
#endif

}


} // wdk

