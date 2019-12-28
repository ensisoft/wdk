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

#include <functional> // for funtion
#include <memory>  // for unique_ptr
#include <utility> // for pair
#include <string>

#include "callback.h"
#include "utility.h"
#include "types.h"

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

    class Window
    {
    public:
        // character encoding for char events. defaults to utf8
        enum class Encoding {
            ASCII, UCS2, UTF8
        };

        // event callbacks

        // If this build time flag is enabled we can register
        // multiple listeners per each callback.
    #ifdef WDK_MULTIPLE_WINDOW_LISTENERS
        EventCallback<WindowEventCreate>       on_create;
        EventCallback<WindowEventPaint>        on_paint;
        EventCallback<WindowEventResize>       on_resize;
        EventCallback<WindowEventFocus>        on_lost_focus;
        EventCallback<WindowEventFocus>        on_gain_focus;
        EventCallback<WindowEventWantClose>    on_want_close;
        EventCallback<WindowEventKeydown>      on_keydown;
        EventCallback<WindowEventKeyup>        on_keyup;
        EventCallback<WindowEventChar>         on_char;
        EventCallback<WindowEventMouseMove>    on_mouse_move;
        EventCallback<WindowEventMousePress>   on_mouse_press;
        EventCallback<WindowEventMouseRelease> on_mouse_release;
    #else
        std::function<void (const WindowEventCreate&)>       on_create;
        std::function<void (const WindowEventPaint&)>        on_paint;
        std::function<void (const WindowEventResize&)>       on_resize;
        std::function<void (const WindowEventFocus&)>        on_lost_focus;
        std::function<void (const WindowEventFocus&)>        on_gain_focus;
        std::function<void (const WindowEventWantClose&)>    on_want_close;
        std::function<void (const WindowEventKeydown&)>      on_keydown;
        std::function<void (const WindowEventKeyup&)>        on_keyup;
        std::function<void (const WindowEventChar&)>         on_char;
        std::function<void (const WindowEventMouseMove&)>    on_mouse_move;
        std::function<void (const WindowEventMousePress&)>   on_mouse_press;
        std::function<void (const WindowEventMouseRelease&)> on_mouse_release;
    #endif

        Window();
       ~Window();

        // create the window with the given dimension and flags.
        // window must not exist before.
        //
        // If you're planning on using this window for OpenGL drawing
        // you should pass in a visual id that identifies your OpenGL ćonfiguration.
        // If visualid is 0 the window may not be compatible with your opengl config.
        void Create(const std::string& title, uint_t width, uint_t height, uint_t visualid,
            bool can_resize = true, bool has_border = true, bool initially_visible = true);

        // hide the window if currently visible. (shown)
        void Hide();

        // show the window if currently hidden.
        void Show();

        // destroy the window. window must have been created before.
        void Destroy();

        // invalide the window contents.
        // erases the window contents with the background brush and eventually
        // generates a paint event.
        void Invalidate();

        // move window to x,y position with respect to it's parent. (desktop)
        // precondition: not fullscreen
        // precondition: window has been created
        void Move(int x, int y);

        // toggle between fullscreen/windowed mode.
        void SetFullscreen(bool fullscreen);

        // set input focus to this window
        void SetFocus();

        // set new drawable surface size
        void SetSize(uint_t width, uint_t height);

        // set new character encoding for character events
        void SetEncoding(Encoding e);

        // process the given event.
        // returns true if event was consumed otherwise false.
        bool ProcessEvent(const native_event_t& ev);

        // get the current drawable window surface height
        uint_t GetSurfaceHeight() const;

        // get the current drawable window surface width
        uint_t GetSurfaceWidth() const;

        // returns true if window currently exists. otherwise false.
        bool DoesExist() const;

        // Returns true if window is currently in fullscreen mode
        // otherwise returns false.
        bool IsFullscreen() const;

        // get the current character encoding. the default is utf8
        Encoding GetEncoding() const;

        // get native window handle
        native_window_t GetNativeHandle() const;

        // Get the dimensions for the window with smallest possible
        // dimensions.
        std::pair<uint_t, uint_t> GetMinSize() const;
        // Get the dimensions for the window with biggest possible
        // dimensions.
        std::pair<uint_t, uint_t> GetMaxSize() const;
    private:
        struct impl;

        std::unique_ptr<impl> pimpl_;
    };

} // wdk
