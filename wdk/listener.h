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
    struct WindowEventLostFocus;
    struct WindowEventGainFocus;
    struct WindowEventWantClose;
    struct WindowEventKeyUp;
    struct WindowEventKeyDown;
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
        virtual void OnLostFocus(const WindowEventLostFocus&) {}
        // Invoked on WindowEventFocus message when window gained input focus.
        virtual void OnGainFocus(const WindowEventGainFocus&) {}
        // Invoked on WindowEventWantClose when user wants to close the window.
        virtual void OnWantClose(const WindowEventWantClose&) {}
        // Invoked on WindowEventKeyDown message.
        virtual void OnKeyDown(const WindowEventKeyDown&) {}
        // Invoked on WindowEventKeyUp message.
        virtual void OnKeyUp(const WindowEventKeyUp&) {}
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
    void Disconnect(wdk::Window& window);

    // ugly set of Dispatch overloads for cases when one wants to generically
    // dispatch a window event to a listener.
    inline void Dispatch(const WindowEventCreate& event, WindowListener& listener)
    { listener.OnCreate(event); }
    inline void Dispatch(const WindowEventPaint& event, WindowListener& listener)
    { listener.OnPaint(event); }
    inline void Dispatch(const WindowEventResize& event, WindowListener& listener)
    { listener.OnResize(event); }
    inline void Dispatch(const WindowEventGainFocus& event, WindowListener& listener)
    { listener.OnGainFocus(event); }
    inline void Dispatch(const WindowEventLostFocus& event, WindowListener& listener)
    { listener.OnLostFocus(event); }
    inline void Dispatch(const WindowEventWantClose& event, WindowListener& listener)
    { listener.OnWantClose(event); }
    inline void Dispatch(const WindowEventKeyDown& event, WindowListener& listener)
    { listener.OnKeyDown(event); }
    inline void Dispatch(const WindowEventKeyUp& event, WindowListener& listener)
    { listener.OnKeyUp(event); }
    inline void Dispatch(const WindowEventChar& event, WindowListener& listener)
    { listener.OnChar(event); }
    inline void Dispatch(const WindowEventMouseMove& event, WindowListener& listener)
    { listener.OnMouseMove(event); }
    inline void Dispatch(const WindowEventMousePress& event, WindowListener& listener)
    { listener.OnMousePress(event); }
    inline void Dispatch(const WindowEventMouseRelease& event, WindowListener& listener)
    { listener.OnMouseRelease(event); }

} // wdk

