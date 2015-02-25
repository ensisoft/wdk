WDK (Window Development Kit)
================================

WDK is a minimalistic library to knock up a window for OpenGL rendering.
It also provides simple input handling for mouse (TBD) and keyboard. 
Code requires a C++11 compatible compiler and should compile with
the following compilers:

    o gcc   >= 4.8.1 
    o clang >= 3.4.2 
    o msvc  >= 2013 Express RC

1. Building the library
--------------------------------

For the impatient on Linux:

    $ make 


Building with boost.build: 

1. Install and prepare boost and boost.build:

    $ wget http://sourceforge.net/projects/boost/files/boost/1.51.0/boost_1_51_0.zip/download boost_1_51_0.zip
    $ unzip boost_15_0.zip
    $ cd boost_1_51_0/tools/build/v2
    $ ./bootstrap.sh
    $ sudo ./b2 install
    $ bjam --version
      Boost.build.2011.12-svn

2. Build WDK:

    $ bjam 


2. How to use the library?
--------------------------------

Typical simple usage scenario:

    // 1. create opengl context with default configuration and GL version 3.2.
    wdk::opengl gl(3, 2);
    
    // 2. create rendering window.
    wdk::window win;
    win.create("My Window", WIDTH, HEIGHT, gl.visualid());
    
    // 3. attach the window to opengl as the current rendering surface.
    gl.attach(win);
    
    // run the rendering loop
    while (do_render)
    {
       render_frame();

       // 5. process window events (if needed)
       win.process_all_events()

       gl.swap();
    }    
    
    // 6. detach the current rendering surface.
    gl.detach();


3. Todo
---------------------------------

- implement keyup events
- mouse input for win32


