

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


TODO:

- multihead support, probably only works with a single display setup at the moment
- uniform unicode character input, windows provides utf16, X11 ucs-4
- X11 accent characters, combining characters?
- X11 IM input?
- keyboard state functions, for win there is GetKeyboardState, but how about linux?
- mouse support
