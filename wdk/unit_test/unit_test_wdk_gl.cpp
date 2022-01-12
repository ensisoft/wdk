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

#ifdef TEST_GLES
#  include <GLES2/gl2.h>
#else
#  include "glcorearb.h"
#endif

#include <thread>

#include "wdk/opengl/config.h"
#include "wdk/opengl/context.h"
#include "wdk/opengl/surface.h"
#include "wdk/system.h"
#include "wdk/window.h"
#include "wdk/pixmap.h"
#include "test_minimal.h"

using namespace wdk;

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
PFNGLDELETEPROGRAMPROC           DeleteProgram;
PFNGLREADPIXELSPROC              ReadPixels;
} // gl

template<typename T>
T resolve(const char* name, const wdk::Context& opengl)
{
    T ret = (T)opengl.Resolve(name);
    TEST_REQUIRE(ret != nullptr);
    return ret;
}

#define RESOLVE(x) gl::x = resolve<decltype(gl::x)>("gl"#x, opengl)

void TestResolveEntryPoints(const wdk::Context& opengl)
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
    RESOLVE(DeleteProgram);
    RESOLVE(ReadPixels);
}

void unit_test_config()
{
    // Test creating a config with "don't care" attributes
    {
        Config c(Config::DONT_CARE);
        Context ctx(c);
    }

    // Test creating a config with default attributes
    {
        Config c(Config::DEFAULT);
        Context ctx(c);
    }

    // Test creating a config with some "reasonable" attributes.
    // Note: that supposedly this could also fail depending on the 
    // implementation but we expect this not to be the case.
    {
        Config::Attributes attrs;
        attrs.red_size   = 8;
        attrs.green_size = 8;
        attrs.blue_size  = 8;
        attrs.depth_size = 16;
        attrs.stencil_size = 8;
        attrs.double_buffer = true;
        Config conf(attrs);
        Context ctx(conf);
    }

    // test that creating a config with weird (unsupported attributes) fails
    // and throws an exception.
    try
    {
        Config::Attributes attrs;
        attrs.red_size = 9;
        attrs.green_size = 7;
        attrs.blue_size = 3;

        Config c(attrs);
        TEST_REQUIRE(!"incorrect configuration didn't fail as expected");
    }
    catch ( const std::exception&)
    { /* success*/ }
}

void unit_test_context_should_pass()
{
    // test some context creations that are expected to pass
    {
#ifdef TEST_GLES
        Context ctx_2(Config::DONT_CARE, 2, 0, false);
#else
        Context ctx_1_1(Config::DONT_CARE, 1, 1, false);
        Context ctx_2_0(Config::DONT_CARE, 2, 0, false);
        Context ctx_2_1(Config::DONT_CARE, 2, 1, false);
        Context ctx_3_0(Config::DONT_CARE, 3, 0, false);
#endif
    }

    // test some context creations that are expected to pass
    {
#ifdef TEST_GLES
        Context ctx_2(Config::DEFAULT, 2, 0, false);
#else
        Context ctx_1_1(Config::DEFAULT, 1, 1, false);
        Context ctx_2_0(Config::DEFAULT, 2, 0, false);
        Context ctx_2_1(Config::DEFAULT, 2, 1, false);
        Context ctx_3_0(Config::DEFAULT, 3, 0, false);
#endif
    }
}

void unit_test_context_might_pass()
{
    // these might fail or pass depending on your hardware
    try 
    {
#ifdef TEST_GLES
        Context ctx_1(Config::DONT_CARE, 1, 0, false);
        std::printf("GL ES 1.0 Context, pass\n");
        Context ctx_3_1(Config::DONT_CARE, 3, 1, false);
        std::printf("GL ES 3.1 Context, pass\n");        
#else
        Context ctx_4_6(Config::DONT_CARE, 4, 6, false);  
        std::printf("GL 4.6 Context, pass\n");
        Context ctx_es_1_0(Config::DONT_CARE, 1, 0, false, Context::Type::OpenGL_ES);
        std::printf("GL ES 1.0 Context, pass\n");        
        Context ctx_es_2_0(Config::DONT_CARE, 2, 0, false, Context::Type::OpenGL_ES);
        std::printf("GL ES 2.0 Context, pass\n");
        Context ctx_es_3_1(Config::DONT_CARE, 3, 1, false,  Context::Type::OpenGL_ES);
        std::printf("GL ES 3.1 ES Context, pass\n");
#endif
    }
    catch (const std::exception& e) 
    {}

    // these might fail or pass depending on your hardware
    try 
    {
#ifdef TEST_GLES
        Context ctx_1(Config::DEFAULT, 1, 0, false);
        std::printf("GL ES 1.0 Context, pass\n");        
        Context ctx_3_1(Config::DEFAULT, 3, 1, false);
        std::printf("GL ES 3.1 Context, pass\n");
        Context ctx_3_2(Config::DEFAULT, 3, 2, false);
        std::printf("GL ES 3.2 Context, pass\n");
#else
        Context ctx_4_6(Config::DEFAULT, 4, 6, false);      
        std::printf("GL 4.6 Context, pass\n");        
        Context ctx_es_1_0(Config::DONT_CARE, 1, 0, false, Context::Type::OpenGL_ES);
        std::printf("GL ES 1.0 Context, pass\n");                
        Context ctx_es_2_0(Config::DEFAULT, 2, 0, false, Context::Type::OpenGL_ES);
        std::printf("GL ES 2.0 Context, pass\n");        
        Context ctx_es_3_1(Config::DEFAULT, 3, 1, false,  Context::Type::OpenGL_ES);    
        std::printf("GL ES 3.1 Context, pass\n");        
#endif
    } 
    catch (const std::exception& e)
    {}
}

void unit_test_context_should_fail()
{
    // Test creating a context with a version that is not valid.
    // Expected to fail
    try 
    {
        Context ctx(Config::DONT_CARE, 8, 4, false);
        TEST_REQUIRE(!"incorrect context version didn't fail as expected");
    }
    catch(const std::exception&) 
    { /* success */ }

    try 
    {
        Context ctx(Config::DEFAULT, 8, 4, false);
        TEST_REQUIRE(!"incorrect context version didn't fail as expected");
    }
    catch(const std::exception&) 
    { /* success */ }

#if !defined(TEST_GLES)
    try 
    {
        Context ctx(Config::DEFAULT, 8, 4, false, Context::Type::OpenGL_ES);
        TEST_REQUIRE(!"incorrect context version didn't fail as expected");
    }
    catch(const std::exception&) 
    { /* success */ }  
#endif  
}

#define GL_CHECK(statement) \
    statement; \
    do { \
        const int err = gl::GetError();\
        TEST_REQUIRE(err == GL_NO_ERROR && #statement);\
    } while (0)

void TestRenderQuad(int width, int height)
{
    GLint program = gl::CreateProgram();
    GLuint vert = gl::CreateShader(GL_VERTEX_SHADER);
    GLuint frag = gl::CreateShader(GL_FRAGMENT_SHADER);

#if defined(TEST_GLES)
        const char* v_src = 
          "precision highp float;                                       \n"
          "attribute vec2 a_position;                                   \n"
          "void main()                                                  \n"
          "{                                                            \n"
          "   gl_Position = vec4(a_position, 0, 1);                     \n"
          "}                                                            \n"
          "\n";

        const char* f_src = 
          "void main()                                                  \n"
          "{                                                            \n"
          "  gl_FragColor = vec4(1.0, 0, 0, 0);                         \n"
          "}                                                            \n"
          "\n";
#else
        const char* v_src = 
          "#version 130                                                  \n"
          "in vec2 a_position;                                           \n"
          "void main()                                                   \n"
          "{                                                             \n"                                                               
          "   gl_Position = vec4(a_position, 0, 1);                      \n"
          "}                                                             \n"
          "\n";

        const char* f_src = 
          "#version 130                                                  \n"
          "out vec4 outColor;                                            \n"
          "void main()                                                   \n"
          "{                                                             \n"
          "    outColor = vec4(1.0, 0, 0, 0);                            \n"
          "}                                                             \n"
          "\n";
#endif
    struct Vertex {
        float x, y;
    };        
    static const Vertex quad[6] = {
        {-0.5,  0.5}, 
        {-0.5, -0.5}, 
        { 0.5, -0.5},

        { 0.5, -0.5},
        { 0.5,  0.5},
        {-0.5,  0.5}
    };    

    GL_CHECK(gl::ShaderSource(vert, 1, &v_src, NULL));
    GL_CHECK(gl::CompileShader(vert));
    GL_CHECK(gl::ShaderSource(frag, 1, &f_src, NULL));
    GL_CHECK(gl::CompileShader(frag));
    GL_CHECK(gl::AttachShader(program, vert));
    GL_CHECK(gl::AttachShader(program, frag));
    GL_CHECK(gl::LinkProgram(program));
    GL_CHECK(gl::UseProgram(program));

    const auto a_position = gl::GetAttribLocation(program, "a_position");

    GL_CHECK(gl::Viewport(0, 0, width, height));
    GL_CHECK(gl::ClearColor(0, 0, 0.5, 0));
    GL_CHECK(gl::Clear(GL_COLOR_BUFFER_BIT));
    GL_CHECK(gl::VertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), quad));
    GL_CHECK(gl::EnableVertexAttribArray(a_position));
    GL_CHECK(gl::DrawArrays(GL_TRIANGLES, 0, 6));
    GL_CHECK(gl::DeleteShader(vert));
    GL_CHECK(gl::DeleteShader(frag));
    GL_CHECK(gl::DeleteProgram(program));

    struct Pixel {
        unsigned char r, g, b, a;
    };
    std::vector<Pixel> pixels(width * height);
    GL_CHECK(gl::ReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]));

    unsigned num_red_pixels = 0;
    for (auto& p : pixels) 
    {
        if (p.r == 0xff) num_red_pixels++;
    }
    TEST_REQUIRE(num_red_pixels == width/2 * height/2);
}


void unit_test_surfaces(const wdk::Config::Attributes& attrs)
{
    // Render to a window surface
    {
        wdk::Config config(attrs);
        wdk::Context context(config);
        wdk::Window window;
        window.Create("test", 200, 200, config.GetVisualID(), 
            true, // can resize
            true, // has border
            true // intially visible
        );
        wdk::Surface surface(config, window);
        TEST_REQUIRE(surface.GetNativeHandle());
        TEST_REQUIRE(surface.GetWidth() == 200);
        TEST_REQUIRE(surface.GetHeight() == 200);
        context.MakeCurrent(&surface);

        TestResolveEntryPoints(context);
        TestRenderQuad(200, 200);
    }

    // Render to a offscreen buffer
    {
        wdk::Config config(attrs);
        wdk::Context context(config);
        wdk::Surface surface(config, 200, 200);
        TEST_REQUIRE(surface.GetNativeHandle());
        TEST_REQUIRE(surface.GetWidth() == 200);
        TEST_REQUIRE(surface.GetHeight() == 200);
        context.MakeCurrent(&surface);

        TestResolveEntryPoints(context);
        TestRenderQuad(200, 200);
    }

    // Render to a window system provided buffer (pixmap)
    // this is currently not working on Win.
#if !defined(_WIN32)
    {
        wdk::Config config(attrs);
        wdk::Context context(config);
        wdk::Pixmap pixmap(200, 200, config.GetVisualID());
        wdk::Surface surface(config, pixmap);
        TEST_REQUIRE(surface.GetNativeHandle());
        TEST_REQUIRE(surface.GetWidth() == 200);
        TEST_REQUIRE(surface.GetHeight() == 200);
        context.MakeCurrent(&surface);

        TestResolveEntryPoints(context);
        TestRenderQuad(200, 200);
    }
#endif
}

int test_main(int, char*[])
{
    unit_test_config();
    unit_test_context_should_pass();
    unit_test_context_might_pass();
    unit_test_surfaces(wdk::Config::DEFAULT);
   
    wdk::Config::Attributes attrs;
    attrs.red_size = 8;
    attrs.green_size = 8;
    attrs.blue_size = 8;
    attrs.alpha_size = 8;
#ifdef TEST_GLES
    attrs.depth_size = 8;
#else 
    attrs.depth_size = 16;
#endif
    attrs.stencil_size = 8;
    unit_test_surfaces(attrs);
    return 0;
}