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
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
template <> thread_local const std::collate<char> * locale_traits<char>::coll =
	&std::use_facet<std::collate<char>>(std::locale());
//------------------------------------------------------------------------------
namespace tests {
//------------------------------------------------------------------------------
void locale_traits_test()
{
	bool fail = false;

	try {
		lstring s1 = u8"А";
		lstring s2 = u8"Б";
		lstring s3 = u8"Я";

		if( s1 >= s2 || s1 >= s3 || s2 <= s1 || s2 >= s3 || s3 <= s1 || s3 <= s2 )
			throw std::runtime_error("bad locale traits implementation");

	}
	catch (...) {
		fail = true;
	}

    std::cout << "locale traits test " << (fail ? "failed" : "passed") << std::endl;
}
//------------------------------------------------------------------------------
} // namespace tests
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------