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
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
std::string temp_name(std::string dir = std::string(), std::string pfx = std::string());
std::string get_cwd(bool no_back_slash = false);
std::string path2rel(const std::string & path, bool no_back_slash = false);
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
struct directory_reader {
    static const char path_delimiter[];

    std::function<void()> manipulator;

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

    uint64_t atime = 0;
    uint64_t ctime = 0;
    uint64_t mtime = 0;
    uint32_t atime_nsec = 0;
    uint32_t ctime_nsec = 0;
    uint32_t mtime_nsec = 0;
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
    public:
        void reindex(sqlite::database & db, bool modified_only = true);
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
