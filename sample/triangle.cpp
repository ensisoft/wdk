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

#ifdef SAMPLE_GLES
#    define GL_GLEXT_PROTOTYPES
#  include <GLES2/gl2.h>
#else
#  ifndef _WIN32
#    define GL_GLEXT_PROTOTYPES
#  endif
#  include "glcorearb.h"
#endif
#include <wdk/window_listener.h>
#include <wdk/window_events.h>
#include <wdk/window.h>
#include <wdk/system.h>
#include <wdk/opengl/context.h>
#include <wdk/opengl/surface.h>
#include <wdk/opengl/config.h>
#include <wdk/opengl/opengl.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <chrono>
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

#if !defined(SAMPLE_GLES) && !defined(GL_GLEXT_PROTOTYPES)
PFNGLCREATEPROGRAMPROC           glCreateProgram;
PFNGLCREATESHADERPROC            glCreateShader;
PFNGLSHADERSOURCEPROC            glShaderSource;
PFNGLGETERRORPROC                glGetError;
PFNGLCOMPILESHADERPROC           glCompileShader;
PFNGLDETACHSHADERPROC            glAttachShader;
PFNGLDELETESHADERPROC            glDeleteShader;
PFNGLLINKPROGRAMPROC             glLinkProgram;
PFNGLUSEPROGRAMPROC              glUseProgram;
PFNGLVALIDATEPROGRAMPROC         glValidateProgram;
PFNGLCLEARCOLORPROC              glClearColor;
PFNGLCLEARPROC                   glClear;
PFNGLVIEWPORTPROC                glViewport;
PFNGLDRAWARRAYSPROC              glDrawArrays;
PFNGLGETATTRIBLOCATIONPROC       glGetAttribLocation;
PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGETSTRINGPROC               glGetString;
PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
PFNGLUNIFORM1FPROC               glUniform1f;
PFNGLGETPROGRAMIVPROC            glGetProgramiv;
PFNGLGETSHADERIVPROC             glGetShaderiv;
PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;

template<typename T>
T resolve(const char* name, const wdk::opengl& opengl)
{
    printf("Resolving %s ", name);

    T ret = (T)opengl.resolve(name);

    printf(" ... %p\n", (void*)ret);

    return ret;
}

#define RESOLVE(x) x = resolve<decltype(x)>(#x, opengl)

void resolve(const wdk::opengl& opengl)
{
    RESOLVE(glCreateProgram);
    RESOLVE(glCreateShader);
    RESOLVE(glShaderSource);
    RESOLVE(glGetError);
    RESOLVE(glCompileShader);
    RESOLVE(glAttachShader);
    RESOLVE(glDeleteShader);
    RESOLVE(glLinkProgram);
    RESOLVE(glUseProgram);
    RESOLVE(glValidateProgram);
    RESOLVE(glClearColor);
    RESOLVE(glClear);
    RESOLVE(glViewport);
    RESOLVE(glDrawArrays);
    RESOLVE(glGetAttribLocation);
    RESOLVE(glVertexAttribPointer);
    RESOLVE(glEnableVertexAttribArray);
    RESOLVE(glGetString);
    RESOLVE(glGetUniformLocation);
    RESOLVE(glUniform1f);
    RESOLVE(glGetString);
    RESOLVE(glGetProgramiv);
    RESOLVE(glGetShaderiv);
    RESOLVE(glGetProgramInfoLog);
}
#else
void resolve(const wdk::opengl&) {}
#endif

class triangle : public wdk::window_listener
{
public:
    triangle(wdk::window& win) : program_(0), run_(true), win_(win)
    {
        program_ = glCreateProgram();

        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

#if defined(SAMPLE_GLES)
        const char* v_src =
          "#version 100\n"
          "attribute vec2 a_position;                                   \n"
          "uniform float u_rot;                                         \n"
          "void main()                                                  \n"
          "{                                                            \n"
          "   mat2 r = mat2(cos(u_rot), -sin(u_rot),                    \n"
          "                 sin(u_rot), cos(u_rot));                    \n"
          "   vec2 v = r * a_position;                                  \n"
          "   v.x = clamp(v.x, -1.0, 1.0);                              \n"
          "   v.y = clamp(v.y, -1.0, 1.0);                              \n"
          "   gl_Position = vec4(v, 0.0, 1.0);                          \n"
          "}                                                            \n"
          "\n";

        const char* f_src =
          "#version 100\n"
          "precision mediump float;                                     \n"
          "void main()                                                  \n"
          "{                                                            \n"
          "  gl_FragColor = vec4(0.0, 0.8, 0.0, 0.0);                   \n"
          "}                                                            \n"
          "\n";
#else
        const char* v_src =
          "#version 130                                                  \n"
          "in vec2 a_position;                                           \n"
          "uniform float u_rot;"
          "void main()                                                   \n"
          "{                                                             \n"
          "   mat2 r = mat2(cos(u_rot), -sin(u_rot),                     \n"
          "                 sin(u_rot), cos(u_rot));                     \n"
          "   vec2 v = r * a_position;                                   \n"
          "   v.x = clamp(v.x, -1.0, 1.0);                               \n"
          "   v.y = clamp(v.y, -1.0, 1.0);                               \n"
          "   gl_Position = vec4(v, 0.0, 1.0);                           \n"
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
#endif
        GL_CHECK(glShaderSource(vert, 1, &v_src, NULL));
        GL_CHECK(glCompileShader(vert));

        GLint compile = 0;
        GL_CHECK(glGetShaderiv(vert, GL_COMPILE_STATUS, &compile));
        if (compile == 1)
        {
            std::cout << "Vertex shader compiled OK\n";
        }
        else
        {
            std::cout << "Vertex shader compile failed. :(\n";
        }

        GL_CHECK(glShaderSource(frag, 1, &f_src, NULL));
        GL_CHECK(glCompileShader(frag));
        GL_CHECK(glGetShaderiv(frag, GL_COMPILE_STATUS, &compile));
        if (compile == 1)
        {
            std::cout << "Fragment shader compiled OK\n";
        }
        else
        {
            std::cout << "Fragment shader compile failed. :(\n";
        }

        GL_CHECK(glAttachShader(program_, vert));
        GL_CHECK(glAttachShader(program_, frag));
        GL_CHECK(glLinkProgram(program_));

        GLint link = 0;
        GL_CHECK(glGetProgramiv(program_, GL_LINK_STATUS, &link));
        if (link == 1)
        {
            std::cout << "Program linked OK\n";
        }
        else
        {
            std::cout << "Program link failed :(\n";
            std::string info;
            info.resize(1024);
            GL_CHECK(glGetProgramInfoLog(program_, 1024, NULL, &info[0]));
            std::cout << info;
        }

        GL_CHECK(glUseProgram(program_));

        GL_CHECK(glDeleteShader(vert));
        GL_CHECK(glDeleteShader(frag));
    }

    void render()
    {
        typedef std::chrono::steady_clock clock;
#if _MSC_VER == 1800 // VS2013 has this bug
        // msvc2013 returns this type from steady_clock::now
        // instead of time_point<steady_clock> and then there are no conversion
        // operators between these two unrelated types.
        typedef std::chrono::time_point < std::chrono::system_clock > time;
#else
        typedef std::chrono::time_point<clock> time;
#endif

        typedef std::chrono::duration<float> duration;

        static time stamp = clock::now();
        time now = clock::now();

        duration seconds = now - stamp;

        static float rotation;
        const float velocity = 1; // 1radian/s

        rotation += seconds.count() * velocity;

        GL_ERR_CLEAR;

        GL_CHECK(glClearColor(0.0f, 0.0f, 0.2f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        struct vertex {
            float x, y;
        };
        const vertex triangle[3] = {{0, 1}, {-1, -1}, {1, -1}};

        GLint pos = glGetAttribLocation(program_, "a_position");
        GLint rot = glGetUniformLocation(program_, "u_rot");

        GL_CHECK(glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), triangle));
        GL_CHECK(glEnableVertexAttribArray(pos));
        GL_CHECK(glUniform1f(rot, rotation));

        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));

        stamp = now;

    }

    void on_create(const wdk::window_event_create& create)
    {
        GL_CHECK(glViewport(0, 0, create.width, create.height));
    }
    void on_resize(const wdk::window_event_resize& resize)
    {
        GL_CHECK(glViewport(0, 0, resize.width, resize.height));
    }
    void on_keydown(const wdk::window_event_keydown& key)
    {
        if (key.symbol == wdk::keysym::escape)
            run_ = false;
        else if (key.symbol == wdk::keysym::space)
            win_.set_fullscreen(!win_.is_fullscreen());
    }

    bool running() const
    {
        return run_;
    }
private:
    GLint program_;
    bool run_;
    wdk::window& win_;

};


int launch(int argc, char* argv[])
{
    auto msaa = wdk::config::multisampling::none;
    bool srgb = false;

    for (int i=1; i<argc; ++i)
    {
        if (!std::strcmp(argv[i], "--msaa4"))
            msaa = wdk::config::multisampling::msaa4;
        else if (!std::strcmp(argv[i], "--msaa8"))
            msaa = wdk::config::multisampling::msaa8;
        else if (!std::strcmp(argv[i], "--msaa16"))
            msaa = wdk::config::multisampling::msaa16;

        if (!std::strcmp(argv[i], "--srgb"))
          srgb = true;
    }

    // start with opengl with default config
    wdk::config::attributes attr = wdk::config::DEFAULT;
    attr.sampling    = msaa;
    attr.srgb_buffer = srgb;

    wdk::opengl gl(attr);

    // resolve function pointers if needed
    resolve(gl);

    printf("OpenGL initialized:\n%s\n%s\n%s\n", glGetString(GL_VENDOR), glGetString(GL_VERSION), glGetString(GL_RENDERER));

    // rendering window
    wdk::window win;

    // model and event listener
    triangle model(win);

    // listen to the events
    connect(win, model);

    win.create("Triangle", 600, 600, gl.visualid(),
      true, true, true);

    gl.attach(win);

    wdk::native_event_t event;

    while (model.running())
    {
        model.render();

        gl.swap();

        if (wdk::peek_event(event))
            win.process_event(event);
    }

    gl.detach();

    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        launch(argc, argv);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Oops there was a problem...\n";
        std::cerr << e.what();
        return 1;
    }
    return 0;
}



