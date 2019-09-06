#include <memory>
#include <iostream>
#include <cstring>
#include <wdk/opengl/context.h>
#include <wdk/opengl/config.h>

# define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"

void print_integer(const char* name, GLenum e)
{
    GLint value = 0;
    glGetIntegerv(e, &value);
    std::printf("%-20s = %d\n", name, value);
}

int main(int argc, char* argv[])
{
    try
    {
        bool mobile = false;
        bool desktop = false;
        for (int i=1; i<argc; ++i) 
        {
            if (!std::strcmp(argv[i], "--mobile"))
                mobile = true;
            else if (!std::strcmp(argv[i], "--desktop"))
                desktop = true;
        } 
        if (mobile && desktop) 
        {
            std::cout << "Please choose either --mobile or --desktop. Not both\n";
            return 1;
        }
        if (!mobile && !desktop) 
        {
            std::cout << "Please choose either --mobile or --desktop\n";
            return 1;
        }

        const wdk::context::type type = mobile ? 
            wdk::context::type::mobile : wdk::context::type::desktop;
        const auto minor_version = mobile ? 0 : 5;
        const auto major_version = mobile ? 2 : 4; 

        wdk::config::attributes attr = wdk::config::DEFAULT;
        wdk::config conf(attr);        
        auto context = std::make_unique<wdk::context>(conf, major_version, minor_version, false, type); 

        std::printf("OpenGL initialized:\n%s\n%s\n%s\n\n", 
            glGetString(GL_VENDOR), 
            glGetString(GL_VERSION), 
            glGetString(GL_RENDERER));        
        print_integer("GL_MAX_VARYING_VECTORS", GL_MAX_VARYING_VECTORS);
        print_integer("GL_MAX_VARYING_COMPONENTS", GL_MAX_VARYING_COMPONENTS);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Oops there was a problem... \n";
        std::cerr << e.what();
        return 1;
    }
    return 0;
}