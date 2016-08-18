// Copyright (c) 2010 Sami Väisänen, Ensisoft 
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

// $Id: utf8.h,v 1.8 2010/07/25 12:59:51 svaisane Exp $

// Copyright (c) 2010 Ensisoft www.ensisoft.com 
// 

#pragma once

#include <cstdint>
#include <string>
#include <iterator>
#include <type_traits>

namespace enc
{
    template<typename InputIterator, typename OutputIterator>
    void utf8_encode(InputIterator beg, InputIterator end, OutputIterator dest)
    {
        typedef typename std::iterator_traits<InputIterator>::value_type value_type;
        typedef typename std::make_unsigned<value_type>::type unsigned_type;

        // Unicode conversion table
        // number range (4 bytes)| binary representation (octets) 
        // -----------------------------------------------------------
        // 0000 0000 - 0000 007F | 0xxxxxxx                 (US-ASCII) 
        // 0000 0080 - 0000 07FF | 110xxxxx 10xxxxxx
        // 0000 0800 - 0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
        // 0001 0000 - 0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        while (beg != end)
        {
            // maximum size for a Unicode character is 4 bytes
            uint32_t wchar = static_cast<unsigned_type>(*beg++);

            if (wchar <= 0x007F)
            {
                *dest++ = static_cast<char>(wchar);
            }
            else if (wchar >= 0x0080 &&  wchar <= 0x07FF)
            {
                *dest++ = static_cast<char>((wchar >> 6)   | 0xC0);
                *dest++ = static_cast<char>((wchar & 0x3F) | 0x80);
            }
            else if (wchar >= 0x0800 && wchar <= 0xFFFF)
            {
                *dest++ = static_cast<char>((wchar >> 12)  | 0xE0);
                *dest++ = static_cast<char>(((wchar >> 6)  & 0x3F) | 0x80);
                *dest++ = static_cast<char>((wchar & 0x3F) | 0x80);
            }
            else
            {
                *dest++ = static_cast<char>((wchar >> 18) | 0xF0);
                *dest++ = static_cast<char>(((wchar >> 12) & 0x3F) | 0x80);
                *dest++ = static_cast<char>(((wchar >>  6) & 0x3F) | 0x80);
                *dest++ = static_cast<char>((wchar & 0x3F) | 0x80);
            }
        } // while
    }

    inline
    std::string utf8_encode(const std::string& ascii)
    {
        std::string utf8;
        enc::utf8_encode(ascii.begin(), ascii.end(), std::back_inserter(utf8));
        return utf8;
    }
    

    template<typename WCharType, typename InputIterator, typename OutputIterator>
    InputIterator utf8_decode(InputIterator beg, InputIterator end, OutputIterator dest)
    {
        typedef WCharType wc;
        InputIterator pos;
        while (beg != end)
        {
            pos = beg;
            WCharType w = 0;
            switch (*beg & 0xF0)
            { 
                case 0xF0: // 4 byte sequence
                    if (sizeof(WCharType) < 4)
                        return pos;
                    w |= wc(0x03 & *beg) << 3;
                    for (int i=2; i>=0; --i)
                    {
                        if (++beg == end)
                            return pos;
                        w |= wc(0x3F & *beg) << i*6;
                    }
                    *dest++ = w;
                    break;
                case 0xE0: // 3 byte sequence (fits in 16 bits)
                    if (sizeof(WCharType) < 2)
                        return pos;
                    w |= wc(0x0F & *beg) << 12;
                    if (++beg == end)
                        return pos;
                    w |= wc(0x3F & *beg) << 6;
                    if (++beg == end)
                        return pos;
                    w |= 0x3F & *beg;
                    *dest++ = w;
                    break;
                case 0xC0: // 2 byte sequence
                case 0xD0:
                    if (sizeof(WCharType) < 2)
                        return pos;
                    w |= wc(0x1F & *beg) << 6;
                    if (++beg == end)
                        return pos;
                    w |= 0x3F & *beg;
                    if (w < 0x7F)
                        return pos; // illegal sequence, multibyte but just ascii
                    *dest++ = w;
                    break;
                default:   // 1 byte sequnce (ascii)
                    *dest++ = *beg;
                    break;
            }
            ++beg;
        }
        return beg;
    }

    inline
    std::wstring utf8_decode(const std::string& byte_string)
    {
        std::wstring ret;
        utf8_decode<wchar_t>(byte_string.begin(), byte_string.end(), std::back_inserter(ret));
        return ret;
    }

} // enc


