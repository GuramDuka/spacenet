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
#include <sys/types.h>
#ifndef _WIN32
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <regex>
#include <iostream>
//------------------------------------------------------------------------------
#if defined(_S_IFDIR) && !defined(S_IFDIR)
#define S_IFDIR _S_IFDIR
#endif
#if defined(_S_IFREG) && !defined(S_IFREG)
#define S_IFREG _S_IFREG
#endif
//------------------------------------------------------------------------------
#include "config.h"
#include "scope_exit.hpp"
#include "indexer.hpp"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
std::string get_cwd(bool no_back_slash)
{
#if _WIN32
	DWORD a = GetCurrentDirectoryW(0, NULL) + 1;
	std::unique_ptr<wchar_t> dir_name(new wchar_t [a]);
	GetCurrentDirectoryW(a, dir_name.get());
	return booster::nowide::convert(dir_name.get()) + (no_back_slash ? "" : "\\");
#else
	std::string s;

    s.resize(32);

    for(;;) {
		errno = 0;

		if( getcwd(&s[0], s.size() + 1) != nullptr )
			break;

		if( errno != ERANGE ) {
			auto err = errno;
			throw std::runtime_error("Failed to get current work directory, " + std::to_string(err));
		}

		s.resize(s.size() << 1);
    }

    s.resize(strlen(s.c_str()));

    if( s.empty() )
	s = ".";

    if( !no_back_slash )
		s += "/";

	s.shrink_to_fit();

    return s;
#endif
}
//------------------------------------------------------------------------------
std::string path2rel(const std::string & path, bool no_back_slash)
{
	std::string file_path = path.find('/') == 0 ? path.substr(1) : path;
#if _WIN32
	if( !file_path.empty() ) {
		file_path = std::str_replace(file_path, "/", "\\");

		if( no_back_slash ) {
			if( file_path.back() == '\\' )
				file_path.pop_back();
		}
		else if( file_path.back() != '\\' ){
				file_path.push_back('\\');
		}
	}
#else
	if( !file_path.empty() ) {

		if( no_back_slash ) {
			if( file_path.back() == '/' )
				file_path.pop_back();
		}
		else if( file_path.back() != '/' ){
			file_path.push_back('/');
		}
	}
#endif
	return file_path;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
struct file_stat :
#if _WIN32
	public _stat64
#else
	public stat
#endif
{
	file_stat(const std::string & file_name) noexcept {
		stat(file_name);
	}

	int stat(const std::string & file_name) noexcept {
#if _WIN32
		return _wstat64(booster::nowide::convert(file_name).c_str(), this);
#else
		return ::stat(file_name.c_str(), this);
#endif
	}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
const char directory_reader::path_delimiter[] =
#if _WIN32
	"\\"
#else
	"/"
#endif
;
//------------------------------------------------------------------------------
void directory_reader::read(const std::string & root_path)
{
	std::regex mask_regex(mask.empty() ? ".*" : mask);
	std::regex exclude_regex(exclude);

	path = root_path;

	if( path.back() == path_delimiter[0] )
		path.pop_back();

#if _WIN32
	at_scope_exit(
		while( !stack.empty() ) {
			FindClose(stack.top().handle);
			stack.pop();
		}
	);
#else
	at_scope_exit(
		while( !stack.empty() ) {
			::closedir(static_cast<DIR *>(stack.top().handle));
			stack.pop();
		}
	);
#endif

#if _WIN32
	HANDLE handle = INVALID_HANDLE_VALUE;
#else
	DIR * handle = nullptr;
#endif

	for(;;) {
#if _WIN32
		DWORD err;
		WIN32_FIND_DATAW fdw;
#else
		int err;
		struct dirent * ent;
#if HAVE_READDIR_R
		struct dirent * result, res;
		ent = &res;
#endif
#endif

#if _WIN32
		if( handle == INVALID_HANDLE_VALUE ) {
			handle = FindFirstFileW(booster::nowide::convert(path + "\\*").c_str(), &fdw);
#else
		if( handle == nullptr ) {
			handle = ::opendir(path.c_str());
#endif
		}
		else if( stack.empty() ) {
			break;
		}
		else {
			handle = static_cast<DIR *>(stack.top().handle);
			path = stack.top().path;
			stack.pop();
#if _WIN32
			lstrcpyW(fdw.cFileName, L"");
#endif
		}

		level = stack.size() + 1;
#if _WIN32
		if( handle == INVALID_HANDLE_VALUE ) {
			err = GetLastError();
			if( err == ERROR_PATH_NOT_FOUND )
				return;
			throw std::runtime_error("Failed to open directory: " + path + ", " + std::to_string(err));

#else
		if( handle == nullptr ) {
			err = errno;
			if( err == ENOTDIR )
				return;
			throw std::runtime_error("Failed to open directory: " + path + ", " + std::to_string(err));
#endif
		}

#if _WIN32
		at_scope_exit(
			if( handle != INVALID_HANDLE_VALUE )
				FindClose(handle);
		);
#else
		at_scope_exit(
			if( handle != nullptr )
				::closedir(handle);
		);
#endif

#if _WIN32
		do {
			if( lstrcmpW(fdw.cFileName, L"") == 0 )
				continue;
			if( lstrcmpW(fdw.cFileName, L".") == 0 && !list_dot )
				continue;
			if( lstrcmpW(fdw.cFileName, L"..") == 0 && !list_dotdot )
				continue;

			name = booster::nowide::convert(fdw.cFileName);
#elif HAVE_READDIR_R
		for(;;) {
			if( readdir_r(handle, ent, &result) != 0 ) {
				err = errno;
				throw std::runtime_error("Failed to read directory: " + path + ", " + std::to_string(err));
			}

			if( result == nullptr )
				break;

			if( strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0 )
				continue;

			name = ent->d_name;
#else
		for(;;) {
			errno = 0;
			ent = readdir(handle);
            err = errno;

			if( ent == nullptr ) {
                if( err != 0 )
                    throw std::runtime_error("Failed to read directory: " + path + ", " + std::to_string(err));
				break;
			}

			if( strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0 )
				continue;

			name = ent->d_name;
#endif
			bool match = std::regex_match(name, mask_regex);

			if( match && !exclude.empty() )
				match = !std::regex_match(name, exclude_regex);

			path_name = path + path_delimiter + name;
			file_stat fs(path_name);

			if( (err = errno) != 0 )
				throw std::runtime_error("Failed to read file info: " + path_name + ", " + std::to_string(err));

			//if( (fs.st_mode & type_mask) == 0 )
			//	continue;
			atime = fs.st_atime;
			ctime = fs.st_ctime;
			mtime = fs.st_mtime;
			fsize = fs.st_size;
			isfreg = fs.st_mode & S_IFREG;
			islnk = fs.st_mode & S_IFLNK;
#if _WIN32
			if( (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ) {
#elif __USE_MISC
//&& DT_UNKNOWN && DT_DIR && IFTODT
            auto d_type = ent->d_type;

			if( d_type == DT_UNKNOWN ) {
                struct stat st;

                if( ::stat(path_name.c_str(), &st) != 0 ) {
                    err = errno;
                    throw std::runtime_error("Failed to stat entry: " + path_name + ", " + std::to_string(err));
                }

                d_type = IFTODT(st.st_mode);
			}

			if( d_type == DT_DIR ) {
#else
			struct stat st;

			if( ::stat(path_name.c_str(), &st) != 0 ) {
				err = errno;
				throw std::runtime_error("Failed to stat entry: " + path_name + ", " + std::to_string(err));
			}

			if( (st.st_mode & S_IFDIR) != 0 ) {
#endif

                void * data = nullptr;

				if( list_directories && match )
					data = manipulator();

				if( recursive && (max_level == 0 || stack.size() < max_level ) ) {
					stack.push({ handle, data, path });
					path += path_delimiter + name;
#if _WIN32
					handle = INVALID_HANDLE_VALUE;
#else
					handle = nullptr;
#endif
					break;
				}
			}
			else if( match )
				manipulator();
		}
#if _WIN32
		while( FindNextFileW(handle, &fdw) != 0 );

		if( handle != INVALID_HANDLE_VALUE && GetLastError() != ERROR_NO_MORE_FILES ) {
			err = GetLastError();
			throw std::runtime_error("Failed to read directory: " + path + ", " + std::to_string(err));
		}
#endif
	}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void directory_indexer::reindex(bool modified_only)
{
	modified_only = modified_only;

	entry root;
	{
        directory_reader dr;

        dr.recursive = dr.list_directories = true;
        dr.manipulator = [&] {
            entry * parent = static_cast<entry *>(dr.stack.empty() ? &root : dr.stack.top().data);

            parent->lst_.emplace_front(entry(parent, dr.level, dr.name, dr.mtime, dr.fsize, dr.isfreg));

            std::string full_path_name;

            for( auto p = parent; p != &root; p = p->parent_ )
                full_path_name += p->name_ + directory_reader::path_delimiter;
            full_path_name += dr.name;
            std::cout << full_path_name << " " << dr.level << std::endl;

            return &parent->lst_.front();
        };

        dr.read(get_cwd());
    }

	std::function<void(const entry &)> f;

	f = [&] (const entry & parent) {
		for( const auto & e : parent.lst_ ) {
			if( e.isfreg_ ) {
				for( auto p = &parent; p->parent_ != nullptr; p = p->parent_ )
					std::cout << p->name_ << directory_reader::path_delimiter;
				std::cout << e.name_ << std::endl;
			}
			f(e);
		}
	};

	f(root);

}
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
