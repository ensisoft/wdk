

WDK (Window Development Kit) is aimed to be a minimalistic library to knock up a window for OpenGL rendering.
It also provides simple input handling (TBD for mouse) for mouse and keyboard. 


How to use the library?

First choose your desired Open GL configuration and create a context object. Then using the 
context object's visual id you can create a window object compatible with the rendering context.
Once you're ready to render to the window call make_current() on the contex passing handle to
the window you want to use for rendering. 

The library aims to provide classes that can also function standalone as much as possible and can be 
integrated to other code using the native window system apis. Hence the extensive use of handles
in parameters. Care should be taken not to pass invalid handles or not to let object life times 
invalidate handles.


Typical startup:

1. create display connection
2. set config attributes (if needed)
3. create config object
3. create rendering rendering context
4. create window
5. create surface for the window
6. make the surface current in the context
7. start drawing

Typical shutdown:
1. set null for current surface in the context
2. dispose surface
3. close window


Code requires a C++11 compatible compiler. Compiles with gcc >= 4.8.1 and msvc >= 2013 Express RC


TODO:

- multihead support, probably only works with a single display setup at the moment
- uniform unicode character input, windows provides utf16, X11 ucs-4
- X11 accent characters, combining characters?
- X11 IM input?
- keyboard state functions, for win there is GetKeyboardState, but how about linux?
- mouse support
