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
#ifndef INDEXER_HPP_INCLUDED
#define INDEXER_HPP_INCLUDED
//------------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------------
#include <functional>
#include <string>
#include <stack>
#include <forward_list>
//------------------------------------------------------------------------------
#include "config.h"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
std::string get_cwd(bool no_back_slash = false);
std::string path2rel(const std::string & path, bool no_back_slash = false);
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
struct directory_reader {
	static const char path_delimiter[];

	std::function<void * ()> manipulator;

	struct stack_entry {
#if _WIN32
		HANDLE handle;
#else
		void * handle;
#endif
        void * data;
		std::string path;
	};

	std::stack<stack_entry> stack;

	std::string path;
	std::string path_name;
	std::string name;
	std::string mask;
	std::string exclude;
	uintptr_t level = 0;
	uintptr_t max_level = 0;
	bool list_dot = false;
	bool list_dotdot = false;
	bool list_directories = false;
	bool recursive = false;

	time_t atime = 0;
	time_t ctime = 0;
	time_t mtime = 0;
	uint64_t fsize = 0;
	bool isfreg = false;
	bool islnk = false;

	template <typename Manipul>
	void read(const std::string & root_path, const Manipul & ml) {
		this->manipulator = [&] {
			ml();
		};

		read(root_path);
	}

	void read(const std::string & root_path);
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class directory_indexer {
	private:
	protected:
		struct entry {
			entry * parent_ = nullptr;
			std::forward_list<entry> lst_;
			decltype(directory_reader::level) level_ = 0;
			std::string name_;
			decltype(directory_reader::mtime) mtime_ = 0;
			decltype(directory_reader::fsize) size_ = 0;
			decltype(directory_reader::isfreg) isfreg_ = false;

			entry() {}

			entry(
				entry * parent,
				decltype(level_) level,
				decltype(name_) name,
				decltype(mtime_) mtime,
				decltype(size_) fsize,
				decltype(isfreg_) isfreg) :
					parent_(std::move(parent)),
					level_(std::move(level)),
					name_(std::move(name)),
					mtime_(std::move(mtime)),
					size_(std::move(fsize)),
					isfreg_(std::move(isfreg))
				{}
		};

	public:
		void reindex(bool modified_only = true);
};
//------------------------------------------------------------------------------
namespace tests {
//------------------------------------------------------------------------------
void indexer_test();
//------------------------------------------------------------------------------
} // namespace tests
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
#endif // INDEXER_HPP_INCLUDED
//------------------------------------------------------------------------------
