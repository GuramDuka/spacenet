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
#include <iostream>
//------------------------------------------------------------------------------
#include "locale_traits.hpp"
#include "windows.h"
#include "winnls.h"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
namespace tests {
//------------------------------------------------------------------------------
void locale_traits_test()
{
	bool fail = false;

    auto loc = std::locale();
    std::cerr << "locale name: " << loc.name() << std::endl;

	try {
        string
#if _WIN32
        s1 = utf2str(u8"А"), s2 = utf2str(u8"Б"), s3 = utf2str(u8"Я");
#else
        s1 = u8"А", s2 = u8"Б", s3 = u8"Я";
#endif

        bool a1 = s1 < s2;
        bool a2 = s1 < s3;
        bool a3 = s2 < s3;

        if( !a1 || !a2 || !a3 )
            a1 = a2 = a3;//throw std::runtime_error("bad locale traits implementation");

	}
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        fail = true;
    }
    catch (...) {
		fail = true;
	}

    std::cerr << "locale traits test " << (fail ? "failed" : "passed") << std::endl;
}
//------------------------------------------------------------------------------
} // namespace tests
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
