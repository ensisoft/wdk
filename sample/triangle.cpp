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
#  include <GLES2/gl2.h>
#else
#  include "glcorearb.h"
#endif

#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <chrono>
#include <cassert>
#include <cstring>

#include "wdk/listener.h"
#include "wdk/events.h"
#include "wdk/window.h"
#include "wdk/system.h"
#include "wdk/opengl/context.h"
#include "wdk/opengl/surface.h"
#include "wdk/opengl/config.h"
#include "wdk/opengl/opengl.h"


#define GL_ERR_CLEAR \
    while (gl::GetError()) \

#define GL_CHECK(statement) \
    statement; \
    do { \
        const int err = gl::GetError(); \
        if (err != GL_NO_ERROR) { \
            printf("GL error 0x%04x @ %s,%d\n", err, __FILE__, __LINE__); \
            abort(); \
        }\
    } while(0)

namespace gl {
PFNGLCREATEPROGRAMPROC           CreateProgram;
PFNGLCREATESHADERPROC            CreateShader;
PFNGLSHADERSOURCEPROC            ShaderSource;
PFNGLGETERRORPROC                GetError;
PFNGLCOMPILESHADERPROC           CompileShader;
PFNGLDETACHSHADERPROC            AttachShader;
PFNGLDELETESHADERPROC            DeleteShader;
PFNGLLINKPROGRAMPROC             LinkProgram;
PFNGLUSEPROGRAMPROC              UseProgram;
PFNGLVALIDATEPROGRAMPROC         ValidateProgram;
PFNGLCLEARCOLORPROC              ClearColor;
PFNGLCLEARPROC                   Clear;
PFNGLVIEWPORTPROC                Viewport;
PFNGLDRAWARRAYSPROC              DrawArrays;
PFNGLGETATTRIBLOCATIONPROC       GetAttribLocation;
PFNGLVERTEXATTRIBPOINTERPROC     VertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
PFNGLGETSTRINGPROC               GetString;
PFNGLGETUNIFORMLOCATIONPROC      GetUniformLocation;
PFNGLUNIFORM1FPROC               Uniform1f;
PFNGLGETPROGRAMIVPROC            GetProgramiv;
PFNGLGETSHADERIVPROC             GetShaderiv;
PFNGLGETPROGRAMINFOLOGPROC       GetProgramInfoLog;
} // gl

template<typename T>
T resolve(const char* name, const wdk::OpenGL& opengl)
{
    printf("Resolving %s ", name);
    T ret = (T)opengl.Resolve(name);
    printf(" ... %p\n", (void*)ret);
    return ret;
}

#define RESOLVE(x) gl::x = resolve<decltype(gl::x)>("gl"#x, opengl)

void ResolveEntryPoints(const wdk::OpenGL& opengl)
{
    RESOLVE(CreateProgram);
    RESOLVE(CreateShader);
    RESOLVE(ShaderSource);
    RESOLVE(GetError);
    RESOLVE(CompileShader);
    RESOLVE(AttachShader);
    RESOLVE(DeleteShader);
    RESOLVE(LinkProgram);
    RESOLVE(UseProgram);
    RESOLVE(ValidateProgram);
    RESOLVE(ClearColor);
    RESOLVE(Clear);
    RESOLVE(Viewport);
    RESOLVE(DrawArrays);
    RESOLVE(GetAttribLocation);
    RESOLVE(VertexAttribPointer);
    RESOLVE(EnableVertexAttribArray);
    RESOLVE(GetString);
    RESOLVE(GetUniformLocation);
    RESOLVE(Uniform1f);
    RESOLVE(GetString);
    RESOLVE(GetProgramiv);
    RESOLVE(GetShaderiv);
    RESOLVE(GetProgramInfoLog);
}

class RotatingTriangle : public wdk::WindowListener
{
public:
    RotatingTriangle(wdk::Window& win) : mWindow(win)
    {
        mProgram = gl::CreateProgram();
        GLuint vert = gl::CreateShader(GL_VERTEX_SHADER);
        GLuint frag = gl::CreateShader(GL_FRAGMENT_SHADER);

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
        GL_CHECK(gl::ShaderSource(vert, 1, &v_src, NULL));
        GL_CHECK(gl::CompileShader(vert));

        GLint compile = 0;
        GL_CHECK(gl::GetShaderiv(vert, GL_COMPILE_STATUS, &compile));
        if (compile == 1)
        {
            std::cout << "Vertex shader compiled OK\n";
        }
        else
        {
            std::cout << "Vertex shader compile failed. :(\n";
        }

        GL_CHECK(gl::ShaderSource(frag, 1, &f_src, NULL));
        GL_CHECK(gl::CompileShader(frag));
        GL_CHECK(gl::GetShaderiv(frag, GL_COMPILE_STATUS, &compile));
        if (compile == 1)
        {
            std::cout << "Fragment shader compiled OK\n";
        }
        else
        {
            std::cout << "Fragment shader compile failed. :(\n";
        }

        GL_CHECK(gl::AttachShader(mProgram, vert));
        GL_CHECK(gl::AttachShader(mProgram, frag));
        GL_CHECK(gl::LinkProgram(mProgram));

        GLint link = 0;
        GL_CHECK(gl::GetProgramiv(mProgram, GL_LINK_STATUS, &link));
        if (link == 1)
        {
            std::cout << "Program linked OK\n";
        }
        else
        {
            std::cout << "Program link failed :(\n";
            std::string info;
            info.resize(1024);
            GL_CHECK(gl::GetProgramInfoLog(mProgram, 1024, NULL, &info[0]));
            std::cout << info;
        }

        GL_CHECK(gl::UseProgram(mProgram));
        GL_CHECK(gl::DeleteShader(vert));
        GL_CHECK(gl::DeleteShader(frag));
    }

    void Render()
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

        struct vertex {
            float x, y;
        };
        const vertex triangle[3] = {{0, 1}, {-1, -1}, {1, -1}};

        const GLint pos = gl::GetAttribLocation(mProgram, "a_position");
        const GLint rot = gl::GetUniformLocation(mProgram, "u_rot");

        GL_ERR_CLEAR;
        GL_CHECK(gl::ClearColor(0.0f, 0.0f, 0.2f, 1.0f));
        GL_CHECK(gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        GL_CHECK(gl::VertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), triangle));
        GL_CHECK(gl::EnableVertexAttribArray(pos));
        GL_CHECK(gl::Uniform1f(rot, rotation));
        GL_CHECK(gl::DrawArrays(GL_TRIANGLES, 0, 3));

        stamp = now;

    }

    void OnCreate(const wdk::WindowEventCreate& create)
    {
        GL_CHECK(gl::Viewport(0, 0, create.width, create.height));
    }
    void OnResize(const wdk::WindowEventResize& resize)
    {
        GL_CHECK(gl::Viewport(0, 0, resize.width, resize.height));
    }
    void OnKeyDown(const wdk::WindowEventKeyDown& key)
    {
        if (key.symbol == wdk::Keysym::Escape)
            mRunning = false;
        else if (key.symbol == wdk::Keysym::Space)
            mWindow.SetFullscreen(!mWindow.IsFullscreen());
    }

    bool IsRunning() const
    {
        return mRunning;
    }
private:
    GLint mProgram = 0;
    bool mRunning  = true;
    wdk::Window& mWindow;

};

// returns number of seconds elapsed since the last call
// of this function.
double ElapsedSeconds()
{
    using clock = std::chrono::steady_clock;
    static auto start = clock::now();
    auto now  = clock::now();
    auto gone = now - start;
    start = now;
    return std::chrono::duration_cast<std::chrono::microseconds>(gone).count() /
        (1000.0 * 1000.0);
}

int launch(int argc, char* argv[])
{
    auto msaa = wdk::Config::Multisampling::None;
    bool srgb = true;
    int swap_interval = 0;

    for (int i=1; i<argc; ++i)
    {
        if (!std::strcmp(argv[i], "--msaa4"))
            msaa = wdk::Config::Multisampling::MSAA4;
        else if (!std::strcmp(argv[i], "--msaa8"))
            msaa = wdk::Config::Multisampling::MSAA8;
        else if (!std::strcmp(argv[i], "--msaa16"))
            msaa = wdk::Config::Multisampling::MSAA16;
        else if (!std::strcmp(argv[i], "--sync"))
            swap_interval = 1;

        if (!std::strcmp(argv[i], "--no-srgb"))
          srgb = false;
    }

    // start with opengl with default config
    wdk::Config::Attributes attr = wdk::Config::DEFAULT;
    attr.sampling    = msaa;
    attr.srgb_buffer = srgb;

    wdk::OpenGL gl(attr);

    // resolve function pointers
    ResolveEntryPoints(gl);

    printf("OpenGL initialized:\n%s\n%s\n%s\n",
        gl::GetString(GL_VENDOR),
        gl::GetString(GL_VERSION),
        gl::GetString(GL_RENDERER));

    // rendering window
    wdk::Window win;

    // model and event listener
    RotatingTriangle model(win);

    // listen to the events
    Connect(win, model);

    win.Create("Triangle", 600, 600, gl.GetVisualID(),
      true, true, true);

    gl.Attach(win);
    printf("Set swap interval to: %d, %s\n",
        swap_interval, gl.SetSwapInterval(swap_interval) ? "Success" : "Fail");

    wdk::native_event_t event;

    while (model.IsRunning())
    {
        model.Render();

        gl.SwapBuffers();

        if (wdk::PeekEvent(event))
            win.ProcessEvent(event);

        // do some simple statistics bookkeeping.
        static auto frames_total = 0;
        static auto frames       = 0;
        static auto seconds      = 0.0;

        frames_total += 1;
        frames  += 1;
        seconds += ElapsedSeconds();
        if (seconds > 1.0)
        {
            const auto fps = frames / seconds;
            printf("\rFPS: %f", fps);
            fflush(stdout);
            frames  = 0;
            seconds = 0.0;
        }
    }

    gl.Detach();

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



