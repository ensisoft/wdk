WDK (Window Development Kit)
================================

WDK is a minimalistic library to knock up a window for OpenGL rendering.
It also provides simple input handling for mouse and keyboard, fullscreen windows and changing the system resolution. (Probably doesn't work in case of multiple monitors, patches welcome!)
Code requires a C++11 compatible compiler and should compile with
the following compilers:

    * gcc   >= 4.8.1
    * clang >= 3.4.2
    * msvc  >= 2013 Express RC

![Screenshot](https://raw.githubusercontent.com/ensisoft/wdk/master/screens/triangle.png "Triangle demo")

Supported Platforms and Features
--------------------------------

The following APIs are supported:
* Windows: EGL, WGL
* Linux:   EGL, GLX

You can use the library to create both "big desktop" GL and "mobile" GL ES rendering contexts. 
The default build will create two libraries one that links against the desktop windowing system library
(GLX on Linux and WGL on Windows) and creates a "desktop" lib. The other library will link against EGL
and will create a "mobile" lib.

Both WGL and GLX can be used to also create GL ES context if the appropriate extensions are found:
* GLX_EXT_create_context_es2_profile
* WGL_EXT_create_context_es2_profile

Currently the inverse is not possible, i.e it's not possible to use EGL to create "big desktop" GL contexts.

The following rendering surfaces are supported:
* GLX: window, pbuffer, pixmap
* EGL: window, pbuffer, pixmap
* WGL: window, pbuffer

Note that pbuffer support on Windows requires WGL_ARB_pbuffer extension.

1. Building the library
--------------------------------

You need to have CMake installed.
You'll also need GL headers + runtime and possibly EGL/GLESv2 for the mobile version.

For Linux:

```
  $ git clone https://github.com/ensisoft/wdk
  $ cd wdk
  $ mkdir build
  $ cd build
  $ cmake -G "Unix Makefiles" ..
  $ make
  $ bin/DesktopGLSample
```

For Windows

```
  $ git clone https://github.com/ensisoft/wdk
  $ cd wdk
  $ mkdir build
  $ cd build
  $ cmake -G "Visual Studio 14 2015" ..
  $ msbuild ALL_BUILD.vcxproj /m
  $ bin\DesktopGLSample.exe
```

2. How to use the library?
--------------------------------

Typical simple usage scenario:

```
    // 1. Decide on frame buffer configuration.
    wdk::Config::Attributes attrs = wdk::Config::DEFAULT;
    attrs.red_size = 8;
    attrs.stencil_size = 8;
    // ...

    // 2. select a configuration uusing the attributes
    wdk::Config conf(attrs);

    // 3. create OpenGL context with the config and opengl version
    wdk::OpenGL gl(attrs, 3, 2, false);

    // The calling thread now has a OpenGL rendering context
    // and can proceed to make GL API calls, but it can't really
    // draw yet since the context doesn't have a drawing surface.

    // 4. create a window
    // Note that you'll need to pass the gl.visualid() to make sure that
    // the window will be usable as a rendering surface.
    wdk::Window win;
    win.Create("My Window", WIDTH, HEIGHT, gl.GetVisualID());

    // 4. attach the window to opengl as the current rendering surface.
    // You can now draw!
    gl.Attach(win);

    // 5. run your rendering/game loop.
    while (do_render)
    {
       render_frame();

       // 6. display what was rendered into the buffer.
       gl.SwapBuffers();

       // 7. you might want to process the window system
       // events every once in a while...
       wdk::native_event_t event;
       while (wdk::PeekEvent(event))
          win.ProcessEvent(event)
    }

    // 8. detach the current rendering surface.
    gl.Detach();
```

3. Quick Intro to OpenGL Context Creation
-----------------------------------------

Each OpenGL implementation comes with a something referred to as configuration.
The configurations specify properties such as the color buffer depth, depth buffer,
stencil and auxiliary buffers. The properties vary and each distinct configuration is
identified by an id. (hint on Linux you can use glxinfo to list available configurations).

The window system (such as X11) provides objects such as Windows and Pixmaps which also
come with different properties and configurations. Note that these objects are not
directly related to OpenGL nor provided by OpenGL but instead are created and managed
by the *window system*. However clearly there's a need for OpenGL to be aware of these
resources to some extent in order to be able to use them when for example rendering to a
window as is commonly done. But in order to do so OpenGL needs to understand the properties
of the window system provided object and make sure that the object is in fact compatible.

This compatibility is achieved through a shared ID called a "visual id". Visual id is a
an ID (just an integer value) that is used by both opengl and window system implementations
to identity common sets of compatible properties. For example when you create a window on X11
with XCreateWindow it has an associated visual id. (X window will choose a visual for you
if you don't explicitly specify one). Likewise each opengl config will also have an associated
visual id with it to identify the visual id that it is compatible with.

So in summary to create a OpenGL rendering context:
- Decice on the OpenGL framebuffer properties and then select a config based on those properties.
  Then Use the config's visual id to create compatible window system objects such as windows.

Implementation notes:
The visualid is not really used on Windows. In fact it will be 0 value. Pixelformat is set
when the Window/Buffer is used as a surface object. But in order to keep the code portable
you should pass the visulid value from the config to the window.

Likewise the config id is not available on WGL and possibly not on Windows EGL implementations.
Choosing a config based on configid will not work portably.

