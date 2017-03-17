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
#include <io.h>
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
#if _WIN32 && _MSC_VER
//------------------------------------------------------------------------------
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
//------------------------------------------------------------------------------
int clock_gettime(int dummy, struct timespec * ct);
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
#if _WIN32
const wchar_t *
#else
const char *
#endif
getenv(const string & var_name);
//------------------------------------------------------------------------------
#if _WIN32
#ifndef F_OK
#define F_OK 0
#endif
#ifndef X_OK
#define X_OK 0
#endif
#ifndef W_OK
#define W_OK 2
#endif
#ifndef R_OK
#define R_OK 4
#endif
#endif
int access(const string & path_name, int mode);
//------------------------------------------------------------------------------
int mkdir(const string & path_name);
string home_path(bool no_back_slash = false);
string temp_path(bool no_back_slash = false);
string temp_name(string dir = string(), string pfx = string());
string get_cwd(bool no_back_slash = false);
string path2rel(const string & path, bool no_back_slash = false);
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
struct directory_reader {
    std::function<void()> manipulator_;

    string path_;
    string path_name_;
    string name_;
    string mask_;
    string exclude_;
    uintptr_t level_ = 0;
    uintptr_t max_level_ = 0;

    bool list_dot_ = false;
    bool list_dotdot_ = false;
    bool list_directories_ = false;
    bool recursive_ = false;

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

    bool abort_ = false;

    template <typename Manipul>
    void read(const string & root_path, const Manipul & ml) {
        this->manipulator_ = [&] {
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
        bool modified_only_ = true;
    protected:
    public:
        const auto & modified_only() const {
            return modified_only_;
        }

        directory_indexer & modified_only(decltype(modified_only_) modified_only) {
            modified_only_ = modified_only;
            return *this;
        }

        void reindex(
            sqlite3pp::database & db,
            const string & dir_path_name,
            bool * p_shutdown = nullptr);
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
