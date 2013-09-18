

#pragma once

#include <X11/Xlib.h>
#include <cstring>
#include <stack>

namespace wdk
{
    // X11 has a stupid callback based error handler where every thread 
    // calls the same global error handling routine which by default exits
    // the process. we use a scoped handler to install an automatically restore
    // the previous handler when we want to change the semantics to something
    // more sensible.

    // todo: maybe make this thread safe?
    class scoped_error_handler 
    {
        typedef int (*error_routine)(Display*, XErrorEvent*);
    public:
        scoped_error_handler() : current_error_code(0), previous_error_handler(nullptr)
        {
            previous_error_handler = (error_routine)XSetErrorHandler(error_callback);

            handlers().push(this);
        }
       ~scoped_error_handler()
        {
            XSetErrorHandler(previous_error_handler);

            handlers().pop();
        }
        bool has_error() const
        {
            return bool(current_error_code != 0);
        }
        int code() const
        {
            return current_error_code;
        }
    private:
        static std::stack<scoped_error_handler*>& handlers()
        {
            static std::stack<scoped_error_handler*> handlers;
            return handlers;
        }
        static int error_callback(Display* dpy, XErrorEvent* err)
        {
            scoped_error_handler* topmost = handlers().top();

            topmost->current_error_code = err->error_code;

            return 0;
        }        

        int current_error_code;
        error_routine previous_error_handler;
    };

    template<typename T>
    class factory
    {
    public:
        factory(Display* dpy) : dpy_(dpy),
            has_error_(false), error_code_(0)
        {
        }
        template<typename FactoryFunc>
        T create(FactoryFunc&& construct_new_t)
        {
            has_error_  = false;
            error_code_ = 0;

            XSync(dpy_, False);

            scoped_error_handler err;

            T ret = construct_new_t(dpy_);

            XSync(dpy_, False);

            if (err.has_error())
            {
                has_error_ = true;
                error_code_ = err.code();                
            }
            return ret;
        }

        bool has_error() const
        {
            return has_error_;
        }
        int error_code() const
        {
            return error_code_;
        }
    private:
        Display* dpy_;
        bool has_error_;
        int  error_code_;
    };


} // wdk