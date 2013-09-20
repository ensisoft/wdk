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

#if defined(MAKE_GL1) || defined(MAKE_GL2)
#  ifndef GL_GLEXT_PROTOTYPES
#    define GL_GLEXT_PROTOTYPES // for mesa
#  endif
#  ifdef _WIN32
#  include <windows.h>
#  endif
#  include <GL/gl.h>
#else
#  include <GLES2/gl2.h>
#endif
#include <wdk/window.h>
#include <wdk/events.h>
#include <wdk/keyboard.h>
#include <wdk/display.h>
#include <wdk/event.h>
#include <wdk/context.h>
#include <wdk/config.h>
#include <wdk/surface.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cassert>
#include <cstring>

#define GL_ERR_CLEAR \
    while (glGetError()) \


#define GL_CHECK(statement) \
    statement; \
    do { \
        const int err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            printf("GL error 0x%04x @ %s,%d\n", err, __FILE__, __LINE__); \
            abort(); \
        }\
    } while(0)    

class triangle 
{
public:
    triangle() : program_(0)
    {

#if defined(MAKE_GL2) || defined(MAKE_GL_ES)
        program_ = glCreateProgram();

        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

#if defined(MAKE_GL2)
        const char* v_src = 
          "#version 130                                                  \n"
          "in vec4 a_position;                                           \n"
          "void main()                                                   \n"
          "{                                                             \n"
          "   gl_Position = a_position;                                  \n"
          "}                                                             \n"
          "\n";

        const char* f_src = 
          "#version 130                                                  \n"
          "out vec4 outColor;                                            \n"
          "void main()                                                   \n"
          "{                                                             \n"
          "    outColor = vec4(0, 0.8, 0, 0);                            \n"
          "}                                                             \n"
          "\n";
#elif defined(MAKE_GL_ES)
        const char* v_src = 
          "precision highp float;                                       \n"
          "attribute vec4 a_position;                                   \n"
          "void main()                                                  \n"
          "{                                                            \n"
          "   gl_Position = a_position;                                 \n"
          "}                                                            \n"
          "\n";

        const char* f_src = 
          "void main()                                                  \n"
          "{                                                            \n"
          "  gl_FragColor = vec4(0, 0.8, 0, 0);                         \n"
          "}                                                            \n"
          "\n";
#endif
        GL_CHECK(glShaderSource(vert, 1, &v_src, NULL));
        GL_CHECK(glCompileShader(vert));

        GL_CHECK(glShaderSource(frag, 1, &f_src, NULL));
        GL_CHECK(glCompileShader(frag));

        GL_CHECK(glAttachShader(program_, vert));
        GL_CHECK(glAttachShader(program_, frag));
        GL_CHECK(glLinkProgram(program_));

        GL_CHECK(glValidateProgram(program_));
        GL_CHECK(glUseProgram(program_));

        GL_CHECK(glDeleteShader(vert));
        GL_CHECK(glDeleteShader(frag));
#endif
    }

    void render()
    {
        GL_ERR_CLEAR;

        GL_CHECK(glClearColor(0, 0, 0, 0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

#if defined(MAKE_GL2) || defined(MAKE_GL_ES)
        struct vertex {
          float x, y, z;
          vertex(float x_, float y_, float z_) : x(x_), y(y_), z(z_)
          {
          }
          vertex() : x(0), y(0), z(0)
          {
          }
        };
        
        vertex triangle[3];
        triangle[0] = vertex(0, 1, 0);
        triangle[1] = vertex(-1, -1, 0);
        triangle[2] = vertex(1, -1, 0);
        
        GLint pos = glGetAttribLocation(program_, "a_position");

        GL_CHECK(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), triangle));
        GL_CHECK(glEnableVertexAttribArray(pos));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
#else
        GL_CHECK(glLoadIdentity());
        GL_CHECK(glColor3f(0, 0.8, 0));

        GL_CHECK(glBegin(GL_TRIANGLES));
          glVertex3f(0.0f, 1.0f, 0.0f);
          glVertex3f(-1.0f, -1.0, 0.0f);
          glVertex3f(1.0f, -1.0f, 0.0f);
        GL_CHECK(glEnd());
#endif
      }
private:
    GLint program_;
};

void handle_window_resize(const wdk::window_event_resize& resize)
{
    glViewport(0, 0, resize.width, resize.height);
}

void handle_window_create(const wdk::window_event_create& create)
{
    glViewport(0, 0, create.width, create.height);
}

struct cmdline {
    bool   print_help;
    bool   render_window;
    bool   render_buffer;
    bool   fullscreen;
    bool   listmodes;
    bool   wnd_border;
    bool   wnd_resize;
    bool   wnd_move;
    int    surface_width;
    int    surface_height;
    wdk::native_vmode_t videomode;
};

bool parse_cmdline(int argc, char* argv[], cmdline& cmd)
{
    for (int i=1; i<argc; ++i)
    {
        const char* name = argv[i];

        if (!strcmp(name, "--help"))
            cmd.print_help = true;
        else if (!strcmp(name, "--render-window"))
            cmd.render_window = true;
        else if (!strcmp(name, "--render-buffer"))
            cmd.render_buffer = true;
        else if (!strcmp(name, "--fullscreen"))
            cmd.fullscreen = true;
        else if (!strcmp(name, "--list-modes"))
            cmd.listmodes = true;
        else if (!strcmp(name, "--wnd-no-border"))
            cmd.wnd_border = false;
        else if (!strcmp(name, "--wnd-no-resize"))
            cmd.wnd_resize = false;
        else if (!strcmp(name, "--wnd-no-move"))
            cmd.wnd_move = false;
        else
        {
            if (!(i + 1 < argc))
                return false;

            long value = atoi(argv[++i]);
            if (!strcmp(name, "--wnd-width"))
                cmd.surface_width = value;
            else if (!strcmp(name, "--wnd-height"))
                cmd.surface_height = value;
            else if (!strcmp(name, "--video-mode"))
                cmd.videomode = static_cast<wdk::native_vmode_t>(value);
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
    // default settings
    cmdline cmd = {0};

    cmd.print_help     = false;
    cmd.render_window  = false;
    cmd.render_buffer  = false;
    cmd.fullscreen     = false;
    cmd.listmodes      = false;
    cmd.wnd_border     = true;
    cmd.wnd_resize     = true;
    cmd.wnd_move       = true;
    cmd.surface_width  = 640;
    cmd.surface_height = 480;
    cmd.videomode      = wdk::DEFAULT_VIDEO_MODE;

    if (!parse_cmdline(argc, argv, cmd))
    {
        std::cerr << "Incorrect command line\n";
        return 1;
    }
    else if (cmd.print_help)
    {
        std::cout 
        << "\n"
        << "--help\t\t\tPrint this help\n"
        << "--render-window\t\tRender into a window (default)\n"
        << "--render-buffer\t\tRender into a pbuffer\n"
        << "--fullscreen\t\tChange into fullscreen\n" 
        << "--list-modes\t\tList available video modes\n"
        << "--wnd-no-border\t\tDisable window border\n" 
        << "--wnd-no-resize\t\tDisable window resizing\n" 
        << "--wnd-no-move\t\tDisable window moving\n"
        << "--wnd-width\t\tWindow width\n"
        << "--wnd-height\t\tWindow height\n"
        << "--video-mode\t\tVideo mode\n\n";
        return 0;
    }

    // create display server connection
    wdk::display disp;

    if (cmd.listmodes)
    {
        const auto modes = disp.list_video_modes();

        std::copy(modes.rbegin(), modes.rend(), std::ostream_iterator<wdk::videomode>(std::cout, "\n"));
        return 0;
    }

    if (cmd.videomode)
        disp.set_video_mode(cmd.videomode);

    if (!cmd.render_window && !cmd.render_buffer)
        cmd.render_window = true;
    
    // select GL framebuffer configuration (default)
    wdk::config conf(disp);

    // create GL context (default GL version)
    wdk::context ctx(disp, conf);

    std::cout 
    << "\n"
    << "OpenGL initialized:\n"
    << glGetString(GL_VENDOR) << "\n"
    << glGetString(GL_VERSION) << "\n"
    << glGetString(GL_RENDERER) << "\n"
    << "Surface: " << cmd.surface_width << "x" << cmd.surface_height << "\n";

    // knock up a window on the screen
    wdk::window win(disp);

    // setup window creation params
    wdk::window::params param;
    param.title      = "Simple GL Window";
    param.width      = cmd.surface_width;
    param.height     = cmd.surface_height;
    param.visualid   = conf.visualid();
    param.fullscreen = cmd.fullscreen;
    param.props      = 0;
    if (cmd.wnd_border)
        param.props |= wdk::window::HAS_BORDER;
    if (cmd.wnd_resize)
        param.props |= wdk::window::CAN_RESIZE;
    if (cmd.wnd_move)
        param.props |= wdk::window::CAN_MOVE;

    win.event_resize = handle_window_resize;
    win.event_create = handle_window_create;
    // finally create it
    win.create(param);

    // set a rendering surface and make it current
    wdk::surface surf(disp, conf, win);

    ctx.make_current(&surf);

    //prepare some keyboard handling
    wdk::keyboard kb(disp);

    kb.event_keydown = [&](const wdk::keyboard_event_keydown& key)
    {
        if (key.symbol == wdk::keysym::escape)
        {
            ctx.make_current(nullptr);
            surf.dispose();
            win.close();
        }
    };

    triangle model;

    // enter rendering loop
    while (win.exists())
    {
        model.render();
        ctx.swap_buffers();

        while (disp.has_event())
        {
            wdk::event e = {0};
            disp.get_event(e);
            if (!win.dispatch_event(e))
                kb.dispatch_event(e);

            dispose(e);
        }            
    }

    return 0;
}



