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

#include <wdk/glwindow.h>
#include <wdk/events.h>
#include <wdk/event.h>
#include <wdk/keyboard.h>
#include <wdk/display.h>
#include <wdk/dispatch.h>
#include <wdk/window_listener.h>
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

#if !defined(SAMPLE_GLES) && !defined(GL_GLEXT_PROTOTYPES)
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC  glCreateShader;
PFNGLSHADERSOURCEPROC  glShaderSource;
PFNGLGETERRORPROC      glGetError;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLDETACHSHADERPROC  glAttachShader;
PFNGLDELETESHADERPROC  glDeleteShader;
PFNGLLINKPROGRAMPROC   glLinkProgram;
PFNGLUSEPROGRAMPROC    glUseProgram;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARPROC glClear;
PFNGLVIEWPORTPROC glViewport;
PFNGLDRAWARRAYSPROC glDrawArrays;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGETSTRINGPROC glGetString;

template<typename T>
T resolve(const char* name)
{
	return (T)wdk::context::resolve(name);
}

#define RESOLVE(x) x = resolve<decltype(x)>(#x)

void resolve()
{
	// RESOLVE(glCreateProgram);
	// RESOLVE(glCreateShader);
	// RESOLVE(glShaderSource);
	// RESOLVE(glGetError);
	// RESOLVE(glCompileShader);
	// RESOLVE(glAttachShader);
}
#else
void resolve() {}
#endif

class triangle 
{
public:
    triangle() : program_(0)
    {
        program_ = glCreateProgram();

        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

#if defined(SAMPLE_GLES)
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
#else
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
    }

    void render()
    {
        GL_ERR_CLEAR;

        GL_CHECK(glClearColor(0, 0, 0, 0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

        struct vertex {
          float x, y, z;
        };        
		const vertex triangle[3] = {{0, 1, 0}, {-1, -1, 0}, {1, -1, 0}};
        
        GLint pos = glGetAttribLocation(program_, "a_position");

        GL_CHECK(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), triangle));
        GL_CHECK(glEnableVertexAttribArray(pos));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
      }
private:
    GLint program_;
};

class listener : public wdk::window_listener 
{
public:
    void on_create(const wdk::window_event_create& create)
    {
        GL_CHECK(glViewport(0, 0, create.width, create.height));
    }
    void on_resize(const wdk::window_event_resize& resize)
    {
        GL_CHECK(glViewport(0, 0, resize.width, resize.height));
    }
private:
};


int main(int argc, char* argv[])
{
     // create display server connection
    wdk::display disp;

    // knock up a window on the screen
    wdk::glwindow window(disp);

    listener l;

    window.set_listener(l);

    window.create(wdk::window_params(640, 480, "Triangle"));

    resolve();

    printf("OpenGL initialized:\n%s\n%s\n%s\n", glGetString(GL_VENDOR), glGetString(GL_VERSION), glGetString(GL_RENDERER));

    //prepare some keyboard handling
    wdk::keyboard keyboard(disp);
    keyboard.event_keydown = [&](const wdk::keyboard_event_keydown& key)
    {
        if (key.symbol == wdk::keysym::escape)
        {
            window.close();
        }
    };

    triangle model;

    // enter rendering loop
    while (window.exists())
    {
        model.render();

        window.swap_buffers();

        dispatch_all(disp, window, keyboard);
    }

    return 0;
}



