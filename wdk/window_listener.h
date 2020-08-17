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

namespace wdk
{
    struct WindowEventCreate;
    struct WindowEventPaint;
    struct WindowEventResize;
    struct WindowEventFocus;
    struct WindowEventWantClose;
    struct WindowEventKeyup;
    struct WindowEventKeydown;    
    struct WindowEventChar;
    struct WindowEventMouseMove;
    struct WindowEventMousePress;
    struct WindowEventMouseRelease;
    class  Window;

    // Interface for listening to window events. 
    class WindowListener
    {
    public:
        virtual ~WindowListener() = default;
        // Invoked on WindowEventCreate message.
        virtual void OnCreate(const WindowEventCreate&) {}
        // Invoked on WindowEventPaint message.
        virtual void OnPaint(const WindowEventPaint&) {}
        // Invoked on WindowEventResize message.
        virtual void OnResize(const WindowEventResize&) {}
        // Invoked on WindowEventFocus message when window lost input focus.
        virtual void OnLostFocus(const WindowEventFocus&) {}
        // Invoked on WindowEventFocus message when window gained input focus.
        virtual void OnGainFocus(const WindowEventFocus&) {}
        // Invoked on WindowEventWantClose when user wants to close the window.
        virtual void OnWantClose(const WindowEventWantClose&) {}
        // Invoked on WindowEventKeydown message. 
        virtual void OnKeydown(const WindowEventKeydown&) {}
        // Invoked on WindowEventKeyp message.
        virtual void OnKeyup(const WindowEventKeyup&) {}
        // Invoked on WindowEventChar message.
        virtual void OnChar(const WindowEventChar&) {}
        // Invoked on WindowEventMouseMove message.
        virtual void OnMouseMove(const WindowEventMouseMove&) {}
        // Invoked on WindowEventMousePress message.
        virtual void OnMousePress(const WindowEventMousePress&) {}
        // Invoked on WindowEventMouseRelease message.
        virtual void OnMouseRelease(const WindowEventMouseRelease&) {}
    protected:
    private:
    };

    // connect all events in the window to the listener
    void Connect(wdk::Window& window, wdk::WindowListener& listener);

} // wdk

