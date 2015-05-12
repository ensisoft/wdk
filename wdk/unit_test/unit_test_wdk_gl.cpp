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

#ifndef _WIN32
#  define GL_GLEXT_PROTOTYPES
#endif

#include <GL/gl.h>

#include <boost/test/minimal.hpp>
#include <wdk/system.h>
#include <wdk/config.h>
#include <wdk/context.h>
#include <wdk/window.h>
#include <wdk/surface.h>
#include <wdk/pixmap.h>

using namespace wdk;

void unit_test_config()
{
    {
        config c(config::DONT_CARE);

        BOOST_REQUIRE(c.configid());
        BOOST_REQUIRE(c.handle());

        context ctx(c);

    }

    {
        config c(config::DEFAULT);
        BOOST_REQUIRE(c.configid());
        BOOST_REQUIRE(c.handle());

        context ctx(c);
    }

    try
    {
        config::attributes attrs = {0};
        attrs.red_size = 9;
        attrs.green_size = 7;
        attrs.blue_size = 3;

        config c(attrs);
        BOOST_REQUIRE(!"incorrect configuration didn't fail as expected");
    }
    catch ( const std::exception& e)
    { }

    {
        config::attributes attrs = {0};
        attrs.red_size   = 8;
        attrs.green_size = 8;
        attrs.blue_size  = 8;
        attrs.double_buffer = true;

        config c(attrs);

        context ctx(c);
    }

}

void unit_test_context()
{

    config conf(config::DONT_CARE);

    {

#ifdef TEST_GLES
        context ctx_1(conf, 1, 0);
        context ctx_2(conf, 2, 0);
#else
        context ctx_1_1(conf, 1, 1, false);
        context ctx_2_0(conf, 2, 0, false);
        context ctx_2_1(conf, 2, 1, false);
        context ctx_3_0(conf, 3, 0, false);

        BOOST_REQUIRE(context::resolve("glBegin"));
        BOOST_REQUIRE(context::resolve("glCreateProgram"));
        BOOST_REQUIRE(!context::resolve("ssofuaf"));
#endif
 
    }
}

#define GL_CHECK(statement) \
    statement; \
    do { \
        const int err = glGetError();\
        BOOST_REQUIRE(err == GL_NO_ERROR && #statement);\
    } while (0)


class triangle
{
public:
    triangle()
    {
        program_ = glCreateProgram();

        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

#if defined(SAMPLE_GLES)
        const char* v_src = 
          "precision highp float;                                       \n"
          "attribute vec2 a_position;                                   \n"
          "uniform float u_rot;                                         \n"
          "void main()                                                  \n"
          "{                                                            \n"
          "   mat2 r = mat2(cos(u_rot), -sin(u_rot),                    \n"
          "                 sin(u_rot), cos(u_rot));                    \n"
          "   vec2 v = r * a_position;                                  \n"
          "   v.x = clamp(v.x, -1, 1);                                  \n"
          "   v.y = clamp(v.y, -1, 1);                                  \n"
          "   gl_Position = vec4(v, 0, 1);                              \n"
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
          "in vec2 a_position;                                           \n"
          "uniform float u_rot;"
          "void main()                                                   \n"
          "{                                                             \n"                                                               
          "   mat2 r = mat2(cos(u_rot), -sin(u_rot),                     \n"                        
          "                 sin(u_rot), cos(u_rot));                     \n"
          "   vec2 v = r * a_position;                                   \n"
          "   v.x = clamp(v.x, -1, 1);                                   \n"
          "   v.y = clamp(v.y, -1, 1);                                   \n"
          "   gl_Position = vec4(v, 0, 1);                               \n"
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

        GL_CHECK(glUseProgram(program_));

        GL_CHECK(glDeleteShader(vert));
        GL_CHECK(glDeleteShader(frag));

        a_position = glGetAttribLocation(program_, "a_position");
        u_rotation = glGetUniformLocation(program_, "u_rot"); 
    }

    void render()
    {
        static float rotation;

        rotation += 0.001;

        GL_CHECK(glClearColor(0, 0, 0, 0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

        struct vertex {
            float x, y;
        };        
        const vertex triangle[3] = {{0, 1}, {-1, -1}, {1, -1}};

        GL_CHECK(glUseProgram(program_));        
        GL_CHECK(glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), triangle));
        GL_CHECK(glEnableVertexAttribArray(a_position));
        GL_CHECK(glUniform1f(u_rotation, rotation));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
    }

private:
    GLint program_;
    GLint a_position;
    GLint u_rotation;
};



void unit_test_surfaces()
{
    config::attributes attrs = {0};
    attrs.red_size = 8;
    attrs.green_size = 8;
    attrs.blue_size = 8;
    attrs.double_buffer = true;

    config conf_window(attrs);

    attrs.double_buffer = false;

    config conf_buffer(attrs);

    context ctx_window(conf_window);
    context ctx_buffer(conf_buffer);

    window win;
    win.create("test", 400, 500, conf_window.visualid());

    pixmap pix(400, 500, conf_buffer.visualid());

    surface win_surface(conf_window, win);
    surface pix_surface(conf_buffer, pix);
    surface pbuf_surface(conf_buffer, 400, 50);

    BOOST_REQUIRE(win_surface.width() == win.surface_width());
    BOOST_REQUIRE(win_surface.height() == win.surface_height());

    BOOST_REQUIRE(pix_surface.width() == 400);
    BOOST_REQUIRE(pix_surface.height() == 500);
    BOOST_REQUIRE(pbuf_surface.width() == 400);
    BOOST_REQUIRE(pbuf_surface.height() == 50);

    win.set_size(200, 200);
    win.sync_all_events();

    BOOST_REQUIRE(win.surface_width() == 200);
    BOOST_REQUIRE(win.surface_height() == 200);
    BOOST_REQUIRE(win_surface.width() == 200);
    BOOST_REQUIRE(win_surface.height() ==200);

    ctx_window.make_current(&win_surface);
    triangle win_model;

    ctx_buffer.make_current(&pix_surface);
    triangle buf_model;

    for (int i=0; i<1000; ++i)
    {
        ctx_buffer.make_current(&pix_surface);
        GL_CHECK(glClearColor(1, 0, 0, 0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
        buf_model.render();

        ctx_buffer.make_current(&pbuf_surface);
        GL_CHECK(glClearColor(0, 1, 0, 0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
        buf_model.render();

        ctx_window.make_current(&win_surface);
        GL_CHECK(glClearColor(0.0001 * i, 0.0001 * i, 0.0001 * i, 0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
        GL_CHECK(glViewport(0, 0, 200, 200));
        win_model.render();
        ctx_window.swap_buffers();
    }

}


int test_main(int, char*[])
{
    unit_test_config();
    unit_test_context();
    unit_test_surfaces();

    return 0;
}