
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
#ifdef SAMPLE_USE_BOOST
#  include <boost/program_options.hpp>
#endif
#include <wdk/window.h>
#include <wdk/events.h>
#include <wdk/keyboard.h>
#include <wdk/display.h>
#include <wdk/context.h>
#include <wdk/event.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cassert>

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
        glShaderSource(vert, 1, &v_src, NULL);
        glCompileShader(vert);

        glShaderSource(frag, 1, &f_src, NULL);
        glCompileShader(frag);

        glAttachShader(program_, vert);
        glAttachShader(program_, frag);
        glLinkProgram(program_);

        glValidateProgram(program_);
        glUseProgram(program_);

        glDeleteShader(vert);
        glDeleteShader(frag);
#endif
    }

    void render()
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

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
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), triangle);
        glEnableVertexAttribArray(pos);
        glDrawArrays(GL_TRIANGLES, 0, 3);
#else
        glLoadIdentity();
        glColor3f(0, 0.8, 0);
        glBegin(GL_TRIANGLES);

        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
        glEnd();
#endif
        const int ret = glGetError();
        if (ret != GL_NO_ERROR)
            printf("GL Error: %d\n", ret);

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


int main(int argc, char* argv[])
{
    // create display server connection
    wdk::display disp;

    // create GL context
    wdk::context context(disp.handle());

    // setup window creation parameters
    wdk::window_param p = {};
    p.title      = "GL rendering example";
    p.width      = 640;
    p.height     = 480;
    p.fullscreen = false;
    p.visualid   = context.visualid();

    bool border    = true;
    bool resize    = true;
    wdk::native_vmode_t vmode = wdk::DEFAULT_VIDEO_MODE;
    
    // parse parameters
#ifdef SAMPLE_USE_BOOST
    namespace po = boost::program_options;

    po::options_description desc("Options");
    desc.add_options()
        ("width",         po::value<wdk::uint_t>(&p.width)->default_value(640),           "Window width")
        ("height",        po::value<wdk::uint_t>(&p.height)->default_value(480),          "Window height")
        ("border",        po::value<bool>(&border)->default_value(true),                  "Border")
        ("resize",        po::value<bool>(&resize)->default_value(true),                  "Resize")
        ("videomode",     po::value<wdk::native_vmode_t>(&vmode)->default_value(wdk::DEFAULT_VIDEO_MODE), "Videomode")
        ("listmodes",     "List available video modes")
        ("fullscreen",    "Fullscreen")
        ("help",          "Print help");
    po::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc;
        return 0;
    } 
    else if (vm.count("listmodes"))
    {
        std::vector<wdk::videomode> modes;
        disp.list_video_modes(modes);
        std::copy(modes.rbegin(),
                  modes.rend(),
                  std::ostream_iterator<wdk::videomode>(std::cout, "\n"));
        return 0;
    }
    if (vm.count("fullscreen"))
        p.fullscreen = true;
#endif

    if (border)
        p.props |= wdk::WP_BORDER;
    if (resize)
        p.props |= wdk::WP_RESIZE;

    // change video mode. 
    if (vmode != wdk::DEFAULT_VIDEO_MODE)
        disp.set_video_mode(vmode);


    // finally create our rendering window
    wdk::window win(disp.handle());
    win.event_resize = handle_window_resize;
    win.event_create = handle_window_create;
    win.create(p);

    // keyboard input
    wdk::keyboard kb(disp.handle());
    kb.event_keydown = [&](const wdk::keyboard_event_keydown& key)
    {
        if (key.symbol == wdk::keysym::escape)
            win.close();
    };

    // render in this window 
    context.make_current(win.handle());

    // create model
    triangle model;
    
    printf("\nOpenGL initialized:\n");
    printf("%s, %s, %s\n", glGetString(GL_VENDOR), glGetString(GL_VERSION), glGetString(GL_RENDERER));
    printf("Direct Rendering: %s\n", (context.has_dri() ? "Yes!" : "No :("));
    printf("Surface: %dx%d\n", p.width, p.height);
    
    while (true)
    {
        while (disp.has_event())
        {
            wdk::event e = {0};
            disp.get_event(e);

            if (!win.dispatch_event(e))
                if (kb.dispatch_event(e))
                    
            dispose(e);
        }

        if (!win.exists())
            break;
        
        model.render();
        context.swap_buffers();
    }
    return 0;
}

