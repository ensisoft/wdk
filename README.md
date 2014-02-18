WDK (Window Development Kit)
================================

WDK is a minimalistic library to knock up a window for OpenGL rendering.
It also provides simple input handling for mouse (TBD) and keyboard. 


How to use the library?

First choose your desired Open GL configuration and create a context object. Then use the same
configuration to create a compatible rendering surface (window, pbuffer, pixmap). Once you're 
ready to render to a buffer call make_current on the context with the surface. 

The library aims to provide classes that can also function standalone as much as possible and can be 
integrated to other code using the native window system apis. Hence the direct access to native object
handles. 

Typical simple startup and shutdown:

    // 1. create opengl context with default configuration
    wdk::opengl gl;
    
    // 2. create rendering window.
    wdk::window win;
    win.create("My Window", WIDTH, HEIGHT, gl.visualid());
    
    // 3. attach to the context as the current rendering surface
    gl.attach(win);
    
    // run the rendering loop
    while (do_render)
    {
       render_frame();
       gl.swap();
    }    
    
    // 4. detach the rendering surface
    gl.detach();


More complicated/detailed startup:

   // 1. setup attributes to select a compatible GL color buffer
   wdk::config::attributes attrs;
   attrs.red_size = 8;
   attrs.green_size = 8;
   // ....
   
   // 2. find/create a matching configuration
   wdk::config config;
   
   // 3. create opengl context
   wdk::context ctx(config);
   
   // 4. create rendering window
   wdk::window win;
   win.create("Window", WIDTH, HEIGHT, config.visualid());
   
   // 5. create new rendering surface
   wdk::surface surf(win);
   
   // 6. attach the window to the context as the current rendering surface
   ctx.make_current(&surf);
   
   // run the rendering loop
   while (do_render)
   {
      render_frame();
      ctx.swap_buffers();
   }

   // 7. tear down
   ctx.make_current(nullptr);
   surf.dispose();
   
   


Code requires a C++11 compatible compiler. Compiles with gcc >= 4.8.1 and msvc >= 2013 Express RC


TODO:

- x11 gravity, position 0, 0 looks like it's bottom left
- keyup is not really working properly
- mouse input
- provide display change event 


other todo:
- multihead support, probably only works with a single display setup at the moment
- X11 IM context?


