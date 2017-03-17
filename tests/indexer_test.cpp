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
            string name_;
			decltype(dr.mtime) mtime_;
			decltype(dr.fsize) size_;
			decltype(dr.is_reg) is_reg_;

			entry() {}
            entry(string name, decltype(mtime_) mtime, decltype(size_) fsize, decltype(dr.is_reg) is_reg) :
				name_(std::move(name)),
				mtime_(std::move(mtime)),
				size_(std::move(fsize)),
				is_reg_(std::move(is_reg))
				{}
		};

		std::vector<entry> lst;

        dr.recursive_ = dr.list_directories_ = true;
        dr.manipulator_ = [&] {
            lst.emplace_back(entry(dr.path_name_, dr.mtime, dr.fsize, dr.is_reg));
			return nullptr;
		};

        dr.read(get_cwd());

		locale_traits<char> comparator;

		std::function<bool (const entry & a, const entry & b)> sorter = [&] (const auto & a, const auto & b) {
			int r = (a.is_reg_ ? 1 : 0) - (b.is_reg_ ? 1 : 0);

			if( r == 0 )
				r = comparator.compare(a.name_, b.name_);

			return r < 0;
		};

		std::sort(lst.begin(), lst.end(), sorter);

		//for( const auto & e : lst )
		//	std::cout << e.name_ << std::endl;

		directory_indexer di;
        //string db_name = temp_path(false) + CPPX_U("indexer_test.sqlite");
        string db_name = temp_name() + CPPX_U(".sqlite");
		
		//sqlite::sqlite_config db_config;
        //db_config.flags = sqlite::OpenFlags::READWRITE | sqlite::OpenFlags::CREATE;
		//db_config.encoding = sqlite::Encoding::UTF8;
		//sqlite::database db(db_name, db_config); // ":memory:"
		//sqlite3_busy_timeout(db.connection().get(), 15000); // milliseconds
        sqlite3pp::database db(str2utf(db_name));

		sqlite3pp::command pragmas(db, R"EOS(
			PRAGMA page_size = 4096;
			PRAGMA journal_mode = WAL;
			PRAGMA count_changes = OFF;
			PRAGMA auto_vacuum = NONE;
			PRAGMA cache_size = -2048;
			PRAGMA synchronous = NORMAL;
			PRAGMA temp_store = MEMORY;
		)EOS");
		
		pragmas.execute_all();
		
        di.reindex(db, get_cwd());

	}
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        fail = true;
    }
    catch (...) {
		fail = true;
	}

    std::cerr << "indexer test " << (fail ? "failed" : "passed") << std::endl;
}
//------------------------------------------------------------------------------
} // namespace tests
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
