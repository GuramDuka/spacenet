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
#ifndef CDC512_HPP_INCLUDED
#define CDC512_HPP_INCLUDED
//------------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------------
#include <cinttypes>
//------------------------------------------------------------------------------
#include "config.h"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
constexpr uint32_t cbe32toh(uint32_t x) {
    if( MACHINE_LITTLE_ENDIAN ) {
        x = ( x               << 16) ^  (x >> 16);
        x = ((x & 0x00ff00ff) <<  8) ^ ((x >>  8) & 0x00ff00ff);
    }
    return x;
}
//------------------------------------------------------------------------------
inline uint32_t vbe32toh(uint32_t x) {
    if( MACHINE_LITTLE_ENDIAN ) {
        x = ( x               << 16) ^  (x >> 16);
        x = ((x & 0x00ff00ff) <<  8) ^ ((x >>  8) & 0x00ff00ff);
    }
    return x;
}
//------------------------------------------------------------------------------
constexpr uint32_t chtobe32(uint32_t x) {
    if( MACHINE_LITTLE_ENDIAN ) {
        x = ( x               << 16) ^  (x >> 16);
        x = ((x & 0x00ff00ff) <<  8) ^ ((x >>  8) & 0x00ff00ff);
    }
    return x;
}
//------------------------------------------------------------------------------
inline uint32_t vhtobe32(uint32_t x) {
    if( MACHINE_LITTLE_ENDIAN ) {
        x = ( x               << 16) ^  (x >> 16);
        x = ((x & 0x00ff00ff) <<  8) ^ ((x >>  8) & 0x00ff00ff);
    }
    return x;
}
//------------------------------------------------------------------------------
constexpr uint64_t cbe64toh(uint64_t x) {
    if( MACHINE_LITTLE_ENDIAN ) {
        //x = ((x & 0x00000000ffffffff) << 32) | ((x >> 32) & 0x00000000ffffffff);
        x = (x << 32) | (x >> 32);
        x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) | ((x >> 16) & UINT64_C(0x0000ffff0000ffff));
        x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) | ((x >>  8) & UINT64_C(0x00ff00ff00ff00ff));
    }
    return x;
}
//------------------------------------------------------------------------------
inline uint64_t vbe64toh(uint64_t x) {
    if( MACHINE_LITTLE_ENDIAN ) {
        //x = ((x & 0x00000000ffffffff) << 32) | ((x >> 32) & 0x00000000ffffffff);
        x = (x << 32) | (x >> 32);
        x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) | ((x >> 16) & UINT64_C(0x0000ffff0000ffff));
        x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) | ((x >>  8) & UINT64_C(0x00ff00ff00ff00ff));
    }
    return x;
}
//------------------------------------------------------------------------------
constexpr uint64_t chtobe64(uint64_t x) {
    if( MACHINE_LITTLE_ENDIAN ) {
        //x = ((x & 0x00000000ffffffff) << 32) | ((x >> 32) & 0x00000000ffffffff);
        x = (x << 32) | (x >> 32);
        x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) | ((x >> 16) & UINT64_C(0x0000ffff0000ffff));
        x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) | ((x >>  8) & UINT64_C(0x00ff00ff00ff00ff));
    }
    return x;
}
//------------------------------------------------------------------------------
inline uint64_t vhtobe64(uint64_t x) {
    if( MACHINE_LITTLE_ENDIAN ) {
        //x = ((x & 0x00000000ffffffff) << 32) | ((x >> 32) & 0x00000000ffffffff);
        x = (x << 32) | (x >> 32);
        x = ((x & UINT64_C(0x0000ffff0000ffff)) << 16) | ((x >> 16) & UINT64_C(0x0000ffff0000ffff));
        x = ((x & UINT64_C(0x00ff00ff00ff00ff)) <<  8) | ((x >>  8) & UINT64_C(0x00ff00ff00ff00ff));
    }
    return x;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
struct cdc512_data {
    uint64_t a, b, c, d, e, f, g, h;
  
    void shuffle();
    void shuffle(const cdc512_data & v);
};
//---------------------------------------------------------------------------
struct cdc512 : public cdc512_data {
    union {
        struct {
            uint64_t ba, bb, bc, bd, be, bf, bg, bh;
        };
        uint8_t digest[sizeof(cdc512_data)];
    };
    uint64_t p;

    cdc512() {
        init();
    }
    
    cdc512(leave_uninitialized_type) {}
    
    template <class InputIt>
    cdc512(InputIt first, InputIt last) {
        init();
        update(first, last);
        finish();
    }
    
    void init();
    void update(const void * data, uintptr_t size);
    void finish();

    template <typename InputIt>
    void update(InputIt first, InputIt last) {
        update(&(*first), (last - first) * sizeof(*first));
    }

    template <typename InputIt, typename Container>
    void update(Container & c, InputIt first, InputIt last) {
        init();
        update(first, last);
        finish();
        c.assign(std::cbegin(digest), std::cend(digest));
    }
    
    auto cbegin() {
        return std::cbegin(digest);
    }

    auto cend() {
        return std::cend(digest);
    }
    
    std::string to_string();
};
//------------------------------------------------------------------------------
namespace tests {
//------------------------------------------------------------------------------
void cdc512_test();
//------------------------------------------------------------------------------
} // namespace tests
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
#endif // CDC512_HPP_INCLUDED
//------------------------------------------------------------------------------
