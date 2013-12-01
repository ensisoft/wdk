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

#pragma once

#include <X11/Xlib.h>
#include <cstring>
#include <stack>

namespace wdk
{
    // replace the current global error handler in this scope
    // with an error handler that grabs the error code and 
    // stores it into a variable for later inspection.    
    class scoped_error_grabber
    {
        typedef int (*error_routine)(Display*, XErrorEvent*);
    public:
        scoped_error_grabber() : current_error_code(0), previous_error_callback(nullptr)
        {
            previous_error_callback = (error_routine)XSetErrorHandler(error_callback);

            grabbers().push(this);
        }
       ~scoped_error_grabber()
        {
            XSetErrorHandler(previous_error_callback);

            grabbers().pop();
        }
        bool has_error() const
        {
            return bool(current_error_code != 0);
        }
        int code() const
        {
            return current_error_code;
        }
        void clear()
        {
            current_error_code = 0;
        }
    private:
        static std::stack<scoped_error_grabber*>& grabbers()
        {
            static std::stack<scoped_error_grabber*> grabbers;
            return grabbers;
        }
        static int error_callback(Display* dpy, XErrorEvent* err)
        {
            scoped_error_grabber* topmost = grabbers().top();

            topmost->current_error_code = err->error_code;

            return 0;
        }        

        int current_error_code;
        error_routine previous_error_callback;
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
        T create(FactoryFunc construct_new_t)
        {
            has_error_  = false;
            error_code_ = 0;

            XSync(dpy_, False);

            scoped_error_grabber grabber;

            T ret = construct_new_t(dpy_);

            XSync(dpy_, False);

            if (grabber.has_error())
            {
                has_error_ = true;
                error_code_ = grabber.code();                
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
