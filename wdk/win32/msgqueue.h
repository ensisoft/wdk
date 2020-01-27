// Copyright (c) 2020 Sami Väisänen, Ensisoft 
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

#include <Windows.h>

#include <queue>

namespace wdk
{
    namespace impl {
        using WindowMessageQueue = std::queue<MSG>;
        inline WindowMessageQueue& GetGlobalWindowMessageQueue()
        {
            static WindowMessageQueue queue;
            return queue;
        }
        inline void PutGlobalWindowMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
        {
            assert(msg != WM_APP + 1);
            auto& queue = GetGlobalWindowMessageQueue();
            if (!queue.empty() && queue.back().message == msg)
                return;

            MSG m = { 0 };
            m.hwnd = hwnd; 
            m.message = msg;
            m.lParam = lp;
            m.wParam = wp;
            GetGlobalWindowMessageQueue().push(m);
            // post a message to message queue so that 
            // GetMessage can return from dispatching messages.
            PostMessage(hwnd, WM_APP + 1, 0, 0);
        }
        inline bool GetGlobalWindowMessage(MSG* out)
        {
            auto& queue = GetGlobalWindowMessageQueue();
            if (queue.empty())
                return false;
            *out = queue.front();
            queue.pop();
            return true;
        }
        inline bool HasGlobalWindowMessage()
        {
            auto& queue = GetGlobalWindowMessageQueue();
            return !queue.empty();
        }
    } // namespace
} // namespace

