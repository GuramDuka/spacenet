/*-
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Guram Duka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
//------------------------------------------------------------------------------
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
//------------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------------
#if _MSC_VER
#pragma execution_character_set("utf-8")
#endif
//------------------------------------------------------------------------------
#ifndef _WIN32
#include <unistd.h>
#endif
//------------------------------------------------------------------------------
#if !defined(HAVE_READDIR_R)
#   if defined(__GNUC_PREREQ)
#       if __GNUC_PREREQ (3,2))
#           define HAVE_READDIR_R (_POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _BSD_SOURCE || _SVID_SOURCE || _POSIX_SOURCE)
#       endif
#   endif
#endif
//------------------------------------------------------------------------------
#if __cplusplus
//------------------------------------------------------------------------------
constexpr bool MACHINE_LITTLE_ENDIAN_F() {
    union {
        unsigned char c;
        unsigned x;
    } u = {};
    u.x = 0;
    u.c = 1;
    //static_assert( u.x != 0, "machine byte order detection falied");
    return u.x < 0x10000u;
}
constexpr bool MACHINE_LITTLE_ENDIAN = noexcept(MACHINE_LITTLE_ENDIAN_F());
constexpr bool MACHINE_BIG_ENDIAN = noexcept(!MACHINE_LITTLE_ENDIAN_F());
//------------------------------------------------------------------------------
namespace spacenet {
struct leave_uninitialized_type {};
constexpr const leave_uninitialized_type leave_uninitialized = {};
}
//------------------------------------------------------------------------------
namespace spacenet { namespace tests { void run_tests(); }}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
#endif // CONFIG_H_INCLUDED
//------------------------------------------------------------------------------
