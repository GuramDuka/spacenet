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
#include <vector>
#include <algorithm>
//------------------------------------------------------------------------------
#include "locale_traits.hpp"
#include "indexer.hpp"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
namespace tests {
//------------------------------------------------------------------------------
void indexer_test()
{
	bool fail = false;

	try {
		directory_reader dr;

		struct entry {
			std::string name_;
			decltype(dr.mtime) mtime_;
			decltype(dr.fsize) size_;
			decltype(dr.isfreg) isfreg_;

			entry(std::string name, decltype(mtime_) mtime, decltype(size_) fsize, decltype(dr.isfreg) isfreg) :
				name_(std::move(name)),
				mtime_(std::move(mtime)),
				size_(std::move(fsize)),
				isfreg_(std::move(isfreg))
				{}
		};

		std::vector<entry> lst;

		dr.recursive = dr.list_directories = true;

		dr.read(get_cwd(), [&] {
			lst.emplace_back(entry(dr.path_name, dr.mtime, dr.fsize, dr.isfreg));
		});

		locale_traits<char> comparator;

		std::function<bool (const entry & a, const entry & b)> sorter = [&] (const auto & a, const auto & b) {
			int r = (a.isfreg_ ? 1 : 0) - (b.isfreg_ ? 1 : 0);

			if( r == 0 )
				r = comparator.compare(a.name_, b.name_);

			return r < 0;
		};

		std::sort(lst.begin(), lst.end(), sorter);

		//for( const auto & e : lst )
		//	std::cout << e.name_ << std::endl;

		directory_indexer di;

		di.reindex();

	}
	catch (...) {
		fail = true;
	}

    std::cout << "indexer test " << (fail ? "failed" : "passed") << std::endl;
}
//------------------------------------------------------------------------------
} // namespace tests
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
