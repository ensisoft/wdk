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

#include <cassert>

#include "videomode.h"
#include "system.h"

namespace wdk
{
    // RAII type to (temporarily) change the current graphics/video mode
    // i.e. the resolution. 
    // Will try to restore the video mode to the current mode when the 
    // object is being destructed.
    class TemporaryVideoModeChange
    {
    public:
        TemporaryVideoModeChange(const TemporaryVideoModeChange&) = delete;

        TemporaryVideoModeChange(const VideoMode& mode = VideoMode())
        {
            original_mode_ = GetCurrentVideoMode();
            if (mode.IsValid())
                SetVideoMode(mode);
        }
       ~TemporaryVideoModeChange()
        {
            RestoreVideoMode();
        }
        void SetVideoMode(const VideoMode& mode)
        {
            assert(mode.IsValid());

            const VideoMode& cur_mode = GetCurrentVideoMode();
            if (cur_mode != mode)
                SetVideoMode(mode);
        }
        void RestoreVideoMode()
        {
            assert(original_mode_.IsValid());

            const VideoMode& cur_mode = GetCurrentVideoMode();
            if (cur_mode != original_mode_)
                SetVideoMode(original_mode_);
        }
        TemporaryVideoModeChange& operator=(const TemporaryVideoModeChange&) = delete;
    private:
        VideoMode original_mode_;
    };

} // wdk
