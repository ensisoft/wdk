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

#include "wdk/callback.h"
#include "wdk/utility.h"
#include "wdk/types.h"

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
        EventCallback<WindowEventCreate>       OnCreate;
        EventCallback<WindowEventPaint>        OnPaint;
        EventCallback<WindowEventResize>       OnResize;
        EventCallback<WindowEventLostFocus>    OnLostFocus;
        EventCallback<WindowEventGainFocus>    OnGainFocus;
        EventCallback<WindowEventWantClose>    OnWantClose;
        EventCallback<WindowEventKeydown>      OnKeyDown;
        EventCallback<WindowEventKeyup>        OnKeyUp;
        EventCallback<WindowEventChar>         OnChar;
        EventCallback<WindowEventMouseMove>    OnMouseMove;
        EventCallback<WindowEventMousePress>   OnMousePress;
        EventCallback<WindowEventMouseRelease> OnMouseRelease;
    #else
        std::function<void (const WindowEventCreate&)>       OnCreate;
        std::function<void (const WindowEventPaint&)>        OnPaint;
        std::function<void (const WindowEventResize&)>       OnResize;
        std::function<void (const WindowEventLostFocus&)>    OnLostFocus;
        std::function<void (const WindowEventGainFocus&)>    OnGainFocus;
        std::function<void (const WindowEventWantClose&)>    OnWantClose;
        std::function<void (const WindowEventKeyDown&)>      OnKeyDown;
        std::function<void (const WindowEventKeyUp&)>        OnKeyUp;
        std::function<void (const WindowEventChar&)>         OnChar;
        std::function<void (const WindowEventMouseMove&)>    OnMouseMove;
        std::function<void (const WindowEventMousePress&)>   OnMousePress;
        std::function<void (const WindowEventMouseRelease&)> OnMouseRelease;
    #endif

        Window();
       ~Window();

        // create the window with the given dimension and flags.
        // window must not exist before.
        //
        // If you're planning on using this window for OpenGL drawing
        // you should pass in a visual id that identifies your OpenGL configuration.
        // If visualid is 0 the window may not be compatible with your opengl config.
        void Create(const std::string& title, uint_t width, uint_t height, uint_t visualid,
            bool can_resize = true, bool has_border = true, bool initially_visible = true);

        // hide the window if currently visible. (shown)
        void Hide();

        // show the window if currently hidden.
        void Show();

        // destroy the window. window must have been created before.
        void Destroy();

        // Invalidate the window contents.
        // Erases the window contents with the background brush and eventually
        // generates a paint event.
        void Invalidate();

        // move window to x,y position with respect to it's parent. (desktop)
        // precondition: not fullscreen
        // precondition: window has been created
        void Move(int x, int y);

        // toggle between fullscreen/windowed mode.
        void SetFullscreen(bool fullscreen);

        // Show or hide this window's mouse cursor. When set to false the mouse
        // cursor is hidden whenever the mouse is over this window. This frees
        // the application (such as a game) to display it's own cursor if it so
        // wishes. When set to true the window system's default mouse cursor
        // for this window is restored.
        void ShowCursor(bool on);

        // Try to grab/ungrab the mouse. When the grabbing is enabled the
        // mouse events are always reported with respect to this window
        // even when the mouse cursor isn't on top of this mouse.
        // Note that this is a feature that is shared between all desktop
        // applications and only a single application at the time can have
        // the mouse grabbed. Thus the attempt to grab might fail in which
        // case false is returned to indicate failure. On success returns true.
        bool GrabMouse(bool on_off);

        // set input focus to this window
        void SetFocus();

        // set new drawable surface size
        void SetSize(uint_t width, uint_t height);

        // set new character encoding for character events
        void SetEncoding(Encoding e);

        // Set new window title to be show in the window's title bar (if it has one).
        // The title should be a UTF-8 encoded string.
        void SetTitle(const std::string& title);

        // process the given event.
        // returns true if event was consumed otherwise false.
        bool ProcessEvent(const native_event_t& ev);

        // get the current drawable window surface height
        uint_t GetSurfaceHeight() const;

        // get the current drawable window surface width
        uint_t GetSurfaceWidth() const;

        // get the window x coordinate relative to its parent (desktop)
        // window's upper left corner.
        int GetPosX() const;

        // get the window y coordinate relative to its parent (desktop)
        // window's upper left corner.
        int GetPosY() const;

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
