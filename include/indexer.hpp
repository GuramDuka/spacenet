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
#include <forward_list>
//------------------------------------------------------------------------------
#include "sqlite/sqlite_modern_cpp.h"
#include "sqlite3pp/sqlite3pp.h"
#include "locale_traits.hpp"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
extern const string::value_type path_delimiter[];
//------------------------------------------------------------------------------
string temp_path(bool no_back_slash = false);
string temp_name(string dir = string(), string pfx = string());
string get_cwd(bool no_back_slash = false);
string path2rel(const string & path, bool no_back_slash = false);
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
struct directory_reader {
    std::function<void()> manipulator;

    string path;
    string path_name;
    string name;
    string mask;
    string exclude;
    uintptr_t level = 0;
    uintptr_t max_level = 0;

    bool list_dot = false;
    bool list_dotdot = false;
    bool list_directories = false;
    bool recursive = false;

    uint64_t atime = 0;
    uint64_t ctime = 0;
    uint64_t mtime = 0;
    uint32_t atime_ns = 0;
    uint32_t ctime_ns = 0;
    uint32_t mtime_ns = 0;
    uint64_t fsize = 0;
    bool is_dir = false;
    bool is_reg = false;
    bool is_lnk = false;

    template <typename Manipul>
    void read(const string & root_path, const Manipul & ml) {
        this->manipulator = [&] {
            ml();
        };

        read(root_path);
    }

    void read(const string & root_path);
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class directory_indexer {
    private:
        bool modified_only_;
    protected:
    public:
        const auto & modified_only() const {
            return modified_only;
        }

        directory_indexer & modified_only(decltype(modified_only_) modified_only) {
            modified_only_ = modified_only;
            return *this;
        }

        void reindex(sqlite3pp::database & db);
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
