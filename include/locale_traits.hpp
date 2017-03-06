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
#ifndef LOCALE_TRAITS_HPP_INCLUDED
#define LOCALE_TRAITS_HPP_INCLUDED
//------------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------------
#include <locale>
#include <string>
#include <type_traits>
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
template <typename CharType>
struct locale_traits : public std::char_traits<CharType> {
	static auto lt(CharType a, CharType b) {
		return coll->compare(&a, &a + 1, &b, &b + 1) < 0;
	}

	static auto compare(const CharType * s1, const CharType * s2, size_t n) {
		return coll->compare(s1, s1 + n, s2, s2 + n);
	}

	//static auto compare(const CharType * s1, size_t n1, const CharType * s2, size_t n2) {
	//	return coll->compare(s1, s1 + n1, s2, s2 + n2);
	//}

	static auto compare(const std::basic_string<CharType> & s1, const std::basic_string<CharType> & s2) {
		return coll->compare(&s1[0], &s1[0] + s1.size(), &s2[0], &s2[0] + s2.size());
	}

	static thread_local const std::collate<CharType> * coll;

//	locale_traits() {
//		if
//#if __cpp_if_constexpr >= 201606
//		constexpr
//#endif
//		( std::is_same<char, CharType>::value ) {
//			coll = &std::use_facet<std::collate<char>>(std::locale());
//		}
//	}
};
//------------------------------------------------------------------------------
typedef std::basic_string<char, locale_traits<char>> lstring;
//typedef std::basic_string<wchar_t, locale_traits<wchar_t>> lwstring;
//------------------------------------------------------------------------------
namespace tests {
//------------------------------------------------------------------------------
void locale_traits_test();
//------------------------------------------------------------------------------
} // namespace tests
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
#endif // LOCALE_TRAITS_HPP_INCLUDED
//------------------------------------------------------------------------------
