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
#include <type_traits>
#include <codecvt>
#include <locale>
#include <string>
#include <sstream>
#include <regex>
#include <cwchar>
//------------------------------------------------------------------------------
#if _WIN32
// Implies UTF-16 encoding.
#define CPPX_WITH_SYSCHAR_PREFIX( lit ) L##lit
#else
// Implies UTF-8 encoding.
#define CPPX_WITH_SYSCHAR_PREFIX( lit ) u8##lit
#endif
#define CPPX_U CPPX_WITH_SYSCHAR_PREFIX
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
template <typename CharType>
struct locale_traits : public std::char_traits<CharType> {
	static auto lt(CharType a, CharType b) {
#if _WIN32
        return std::wcsncmp(&a, &b, 1) < 0;
#else
        return coll->compare(&a, &a + 1, &b, &b + 1) < 0;
#endif
	}

	static auto compare(const CharType * s1, const CharType * s2, size_t n) {
#if _WIN32
        return std::wcsncmp(s1, s2, n);
#else
        return coll->compare(s1, s1 + n, s2, s2 + n);
#endif
	}

	//static auto compare(const CharType * s1, size_t n1, const CharType * s2, size_t n2) {
	//	return coll->compare(s1, s1 + n1, s2, s2 + n2);
	//}

    template <typename T>
    static auto compare(const T & s1, const T & s2) {
#if _WIN32
        return std::wcscmp(&s1[0], &s2[0]);
#else
		return coll->compare(&s1[0], &s1[0] + s1.size(), &s2[0], &s2[0] + s2.size());
#endif
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
#if _WIN32
//------------------------------------------------------------------------------
//typedef std::basic_string<wchar_t, locale_traits<wchar_t>> string;
//typedef std::basic_stringstream<wchar_t, locale_traits<wchar_t>> stringstream;
//typedef std::basic_regex<wchar_t, locale_traits<wchar_t>> regex;
typedef std::wstring string;
typedef std::wstringstream stringstream;
typedef std::wregex regex;
//------------------------------------------------------------------------------
//inline string operator + (const wchar_t * s1, const string & s2) {
//    return string(s1) + s2;
//}
//------------------------------------------------------------------------------
//inline string operator + (const string & s1, const std::wstring & s2) {
//    return s1 + s2;
//}
//------------------------------------------------------------------------------
#else
//------------------------------------------------------------------------------
//typedef std::basic_string<char, locale_traits<char>> string;
//typedef std::basic_stringstream<char, locale_traits<char>> stringstream;
//typedef std::basic_regex<char, locale_traits<char>> regex;
typedef std::string string;
typedef std::stringstream stringstream;
typedef std::regex regex;
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
std::wstring str2wstr(const std::string & str);
std::string wstr2str(const std::wstring & str);
//------------------------------------------------------------------------------
#if _WIN32
//------------------------------------------------------------------------------
std::string str2utf(const string & str);
string utf2str(const std::string & str);
//------------------------------------------------------------------------------
#else
//------------------------------------------------------------------------------
constexpr const std::string & str2utf(const string & str)
{
    return str;
}
//------------------------------------------------------------------------------
constexpr const string & utf2str(const std::string & str)
{
    return str;
}
//------------------------------------------------------------------------------
#endif
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
