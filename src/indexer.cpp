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
#include <fstream>
#include <atomic>
#include <unordered_map>
#include <typeinfo>
#if _WIN32
#include <windows.h>
#endif
//------------------------------------------------------------------------------
#if defined(_S_IFDIR) && !defined(S_IFDIR)
#define S_IFDIR _S_IFDIR
#endif
#if defined(_S_IFREG) && !defined(S_IFREG)
#define S_IFREG _S_IFREG
#endif
#if !defined(S_IFLNK)
#define S_IFLNK 0
#endif
//------------------------------------------------------------------------------
#include "config.h"
#include "scope_exit.hpp"
#include "locale_traits.hpp"
#include "cdc512.hpp"
#include "indexer.hpp"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
const char path_delimiter[] =
#if _WIN32
    "\\"
#else
    "/"
#endif
;
//------------------------------------------------------------------------------
std::string temp_name(std::string dir, std::string pfx)
{
	constexpr int MAXTRIES = 100;
	static std::atomic_int index;
	std::string s;
	int pid = getpid();

	if( dir.empty() )
 #if _WIN32
    {
        DWORD a = GetTempPathW(0, NULL);
        wchar_t * s = static_cast<wchar_t *>(alloca(sizeof(wchar_t) * a));
        GetTempPathW(a, s);
        dir = wstr2str(s);
        if( dir.back() == path_delimiter[0] )
            dir.pop_back();
    }
 #else
        dir = P_tmpdir;
 #endif
    if( pfx.empty() )
		pfx = "temp";

	if( access(dir.c_str(), R_OK | W_OK | X_OK) != 0 )
		return std::string();

	size_t l = dir.size() + 1 + pfx.size() + 3 * (sizeof(int) * 3 + 2) + 1;
	s.resize(l);

	int try_n = 0;
	
	do {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		int n = ts.tv_nsec ^ (uintptr_t) &s[0] ^ (uintptr_t) &s;
		snprintf(&s[0], l, "%s/%s-%d-%d-%x", dir.c_str(), pfx.c_str(), pid, index.fetch_add(1), n);
	} while( !access(s.c_str(), F_OK) && try_n++ < MAXTRIES );

	if( try_n >= MAXTRIES )
		return std::string();

	s.resize(strlen(s.c_str()));
	
	return s;
}
//------------------------------------------------------------------------------
std::string get_cwd(bool no_back_slash)
{
#if _WIN32
	DWORD a = GetCurrentDirectoryW(0, NULL) + 1;
    wchar_t * s = static_cast<wchar_t *>(alloca(sizeof(wchar_t) * a));
    GetCurrentDirectoryW(a, s);
    return wstr2str(s) + (no_back_slash ? "" : "\\");
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
static std::string str_replace(const std::string & subject, const std::string & search, const std::string & replace)
{
    std::string s;
    const std::string::size_type l = search.length();
    std::string::const_iterator sb = search.cbegin(), se = search.cend(),
        b = subject.cbegin(), e = subject.cend(), i;

    for( ;;) {
        i = std::search(b, e, sb, se);

        if( i == e )
            break;

        s.append(b, i).append(replace);
        b = i + l;
    }

    s.append(b, i);

    return s;
}
//------------------------------------------------------------------------------
std::string path2rel(const std::string & path, bool no_back_slash)
{
    std::string file_path;
#if _WIN32
    if( !file_path.empty() ) {
        file_path = str_replace(file_path, "/", "\\");
        file_path = path.find('\\') == 0 ? path.substr(1) : path;

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
        file_path = str_replace(file_path, "\\", "/");
        file_path = path.find('/') == 0 ? path.substr(1) : path;

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
        return _wstat64(str2wstr(file_name).c_str(), this);
#else
		return ::stat(file_name.c_str(), this);
#endif
	}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void directory_reader::read(const std::string & root_path)
{
	std::regex mask_regex(mask.empty() ? ".*" : mask);
	std::regex exclude_regex(exclude);

	path = root_path;

	if( path.back() == path_delimiter[0] )
		path.pop_back();

	struct stack_entry {
#if _WIN32
		HANDLE handle;
#else
		DIR * handle;
#endif
		std::string path;
	};

	std::stack<stack_entry> stack;
	
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
			::closedir(stack.top().handle);
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
            auto fffs = str2wstr(path + "\\*");
            handle = FindFirstFileW(fffs.c_str(), &fdw);
#else
		if( handle == nullptr ) {
			handle = ::opendir(path.c_str());
#endif
		}
		else if( stack.empty() ) {
			break;
		}
		else {
			handle = stack.top().handle;
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

            name = wstr2str(fdw.cFileName);
#elif HAVE_READDIR_R
		for(;;) {
			if( readdir_r(handle, ent, &result) != 0 ) {
				err = errno;
				throw std::runtime_error("Failed to read directory: " + path + ", " + std::to_string(err));
			}

			if( result == nullptr )
				break;

			if( strcmp(ent->d_name, ".") == 0  && !list_dot )
				continue;
			if( strcmp(ent->d_name, "..") == 0 && !list_dotdot )
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

			if( strcmp(ent->d_name, ".") == 0  && !list_dot )
				continue;
			if( strcmp(ent->d_name, "..") == 0 && !list_dotdot )
				continue;

			name = ent->d_name;
#endif
			bool match = std::regex_match(name, mask_regex);

			if( match && !exclude.empty() )
				match = !std::regex_match(name, exclude_regex);

			path_name = path + path_delimiter + name;

#if _WIN32
            fsize = fdw.nFileSizeHigh;
            fsize <<= 32;
            fsize += fdw.nFileSizeLow;
            is_reg = (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
            is_dir = (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            is_lnk = false;

            ULARGE_INTEGER q;

            q.LowPart = fdw.ftLastAccessTime.dwLowDateTime;
            q.HighPart = fdw.ftLastAccessTime.dwHighDateTime;
            atime = q.QuadPart / 10000000;
            atime_nsec = (q.QuadPart % 10000000) * 100;

            q.LowPart = fdw.ftCreationTime.dwLowDateTime;
            q.HighPart = fdw.ftCreationTime.dwHighDateTime;
            ctime = q.QuadPart / 10000000;
            ctime_nsec = (q.QuadPart % 10000000) * 100;

            q.LowPart = fdw.ftLastWriteTime.dwLowDateTime;
            q.HighPart = fdw.ftLastWriteTime.dwHighDateTime;
            mtime = q.QuadPart / 10000000;
            ctime_nsec = (q.QuadPart % 10000000) * 100;
#else
            file_stat fs(path_name);

			if( (err = errno) != 0 )
				throw std::runtime_error("Failed to read file info: " + path_name + ", " + std::to_string(err));

			//if( (fs.st_mode & type_mask) == 0 )
			//	continue;
            atime = fs.st_atime;
            ctime = fs.st_ctime;
            mtime = fs.st_mtime;
#if __USE_XOPEN2K8
			atime_nsec = fs.st_atim.tv_nsec;
			ctime_nsec = fs.st_ctim.tv_nsec;
			mtime_nsec = fs.st_mtim.tv_nsec;
#else
			atime_nsec = fs.st_atimensec;
			ctime_nsec = fs.st_ctimensec;
			mtime_nsec = fs.st_mtimensec;
#endif
			fsize = fs.st_size;
			is_reg = (fs.st_mode & S_IFREG) != 0;
			is_dir = (fs.st_mode & S_IFDIR) != 0;
			is_lnk = (fs.st_mode & S_IFLNK) != 0;
#endif
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

				if( list_directories && match && manipulator )
					manipulator();

				if( recursive && (max_level == 0 || stack.size() < max_level ) ) {
					stack.push({ handle, path });
					path += path_delimiter + name;
#if _WIN32
					handle = INVALID_HANDLE_VALUE;
#else
					handle = nullptr;
#endif
					break;
				}
			}
			else if( match ) {
				if( manipulator )
					manipulator();
			}
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
void directory_indexer::reindex(sqlite::database & db, bool modified_only)
{
	constexpr size_t BLOCK_SIZE = 4096;

	try {
	for( const auto p : {
		R"EOS(
		CREATE TABLE IF NOT EXISTS entries (
			id				BLOB PRIMARY KEY ON CONFLICT ABORT,
			parent_id		BLOB,
			name			TEXT,
			is_dir			INTEGER,
			mtime			INTEGER,	/* INTEGER as Unix Time, the number of seconds since 1970-01-01 00:00:00 UTC. */
			mtime_ns		INTEGER,	/* nanoseconds, if supported else zero or NULL */
			digest			BLOB
		) WITHOUT ROWID)EOS",
		"CREATE UNIQUE INDEX IF NOT EXISTS i1 ON entries (parent_id, id)",
		"CREATE UNIQUE INDEX IF NOT EXISTS i2 ON entries (parent_id, name)",
		R"EOS(
		CREATE TABLE IF NOT EXISTS blocks_digests (
			entry_id		BLOB,
			block_no		INTEGER,
			digest			BLOB
        )/* WITHOUT ROWID*/)EOS",
		"CREATE INDEX IF NOT EXISTS i3 ON blocks_digests (entry_id)",
		"CREATE UNIQUE INDEX IF NOT EXISTS i4 ON blocks_digests (entry_id, block_no)"
		} )
		db << p;

	// get a prepared parsed and ready statment
	auto st_sel_id = db << R"EOS(
		SELECT
			id
		FROM
			entries
		WHERE
			parent_id = :parent_id
			AND name = :name
	)EOS";
	auto st_ins = db << R"EOS(
		INSERT INTO entries (
			id, parent_id, name, is_dir, mtime, mtime_ns, digest
		) VALUES (:id, :parent_id, :name, :is_dir, :mtime, :mtime_ns, :digest)
	)EOS";
	auto st_upd = db << R"EOS(
		UPDATE entries SET
			parent_id = :parent_id, name = :name,
			is_dir = :is_dir, mtime = :mtime, mtime_ns = :mtime_ns,
			digest = :digest
		WHERE
			id = :id
	)EOS";
	auto st_blk_ins = db << R"EOS(
		INSERT INTO blocks_digests (
			entry_id, block_no, digest
		) VALUES (:entry_id, :block_no, :digest)
	)EOS";
	auto st_blk_upd = db << R"EOS(
		UPDATE blocks_digests SET
			block_no = :block_no, digest = :digest
		WHERE
			entry_id = :entry_id
	)EOS";

	modified_only = modified_only;

	typedef std::vector<char> blob;
	
	std::unordered_map<std::string, blob> parents;
	
    directory_reader dr;

    dr.recursive = dr.list_directories = true;
	dr.manipulator = [&] {
		cdc512 ctx;

		blob id, parent_id, digest, block_digest;
		{
			auto pit = parents.find(dr.path);

			if( pit == parents.cend() ) {

				if( dr.level > 1 ) {
					ctx.init();
					ctx.update(dr.path.cbegin(), dr.path.cend());
					ctx.finish();

					id.assign(std::begin(ctx.digest), std::end(ctx.digest));
					
					st_sel_id << std::make_pair("id", id);
					st_sel_id << std::make_pair("name", dr.name);
				}
				else {
					st_sel_id << std::make_pair("id", nullptr);
					st_sel_id << std::make_pair("name", dr.path_name);
				}
				
				st_sel_id >> [&] (const blob & a) {
					parent_id = a;
				};

				if( parent_id.empty() )
					parent_id = std::move(id);
				
				parents.emplace(std::make_pair(dr.path, parent_id));
			}
			else {
				parent_id = pit->second;
			}

			ctx.init();
			ctx.update(dr.path_name.cbegin(), dr.path_name.cend());
			ctx.finish();

			id.assign(std::begin(ctx.digest), std::end(ctx.digest));
		}
		
		std::ifstream in(dr.path_name, std::ios::binary);
		in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		
		ctx.init();

		uint64_t blk_no = 0;
		
		auto update_block_digest = [&] {
			// first if needed bind values to it
			st_blk_ins
				<< std::make_pair("id", id)
				<< std::make_pair("blk_no", blk_no)
				<< std::make_pair("block_digest", block_digest)
			;

			try {
				// there is a convinience operator to execute and reset in one go
				db << "BEGIN /* DEFERRED, IMMEDIATE, EXCLUSIVE */ TRANSACTION";
				st_blk_ins.execute();
				db << "COMMIT TRANSACTION";
			}
			// catch specific exceptions as well
			catch( const sqlite::exceptions::constraint & e ) {
			}
			// if you are trying to catch all sqlite related exceptions
			// make sure to catch them by reference
			catch( sqlite::sqlite_exception & e) {
				std::cerr  << e.get_code() << ": " << e.what() << " during "
					  << e.get_sql() << std::endl;
			}
		};
		
		while( !in.eof() ) {
			blk_no++;

			uint8_t buf[BLOCK_SIZE];
			in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE);
			auto r = in.gcount();
			std::memset(buf + r, 0, BLOCK_SIZE - r);

			cdc512 blk_ctx;

			blk_ctx.init();
			blk_ctx.update(std::cbegin(buf), std::cend(buf));
			blk_ctx.finish();
			
			block_digest.assign(std::begin(blk_ctx.digest), std::end(blk_ctx.digest));
			
			update_block_digest();

			ctx.update(std::cbegin(buf), std::cend(buf));
		}
		//try {
		//}
		//catch (std::ios_base::failure &fail) {
			// handle exception here
		//}

		ctx.finish();

		digest.assign(std::begin(ctx.digest), std::end(ctx.digest));
		
		// first if needed bind values to it
		st_ins
			<< std::make_pair("id", id)
			<< std::make_pair("parent_id", parent_id)
			<< std::make_pair("name", dr.name)
			<< std::make_pair("is_dir", dr.is_dir)
			<< std::make_pair("mtime", dr.mtime)
			<< std::make_pair("mtime_nsec", dr.mtime_nsec)
			<< std::make_pair("digest", digest)
		;

		try {
			db << "BEGIN /* DEFERRED, IMMEDIATE, EXCLUSIVE */ TRANSACTION";
			// there is a convinience operator to execute and reset in one go
			st_ins++;
			db << "COMMIT TRANSACTION";
		}
		// catch specific exceptions as well
		catch( const sqlite::exceptions::constraint & e ) {
		}
		// if you are trying to catch all sqlite related exceptions
		// make sure to catch them by reference
		catch( sqlite::sqlite_exception & e) {
			std::cerr  << e.get_code() << ": " << e.what() << " during "
				  << e.get_sql() << std::endl;
		}
	};
	
    dr.read(get_cwd());
		
	// delete lost records
	db << R"EOS(
		DELETE FROM blocks_digests
		WHERE
			entry_id IN (
				SELECT
					a.entry_id
				FROM
					blocks_digests AS a
					LEFT JOIN entries AS b
					ON a.entry_id = b.id
				WHERE
					b.id IS NULL
			)
	)EOS";
	} catch( sqlite::sqlite_exception & e) {
		std::cerr  << e.get_code() << ": " << e.what() << " during "
			  << e.get_sql() << std::endl;
	}
}
//------------------------------------------------------------------------------
void directory_indexer::reindex(sqlite3pp::database & db, bool modified_only)
{
	constexpr size_t BLOCK_SIZE = 4096;

	{
		sqlite3pp::command ddl_sql(db, R"EOS(
		CREATE TABLE IF NOT EXISTS entries (
			id				BLOB PRIMARY KEY ON CONFLICT ABORT,
			parent_id		BLOB,
			name			TEXT,
			is_dir			INTEGER,
			mtime			INTEGER,	/* INTEGER as Unix Time, the number of seconds since 1970-01-01 00:00:00 UTC. */
			mtime_ns		INTEGER,	/* nanoseconds, if supported else zero or NULL */
			digest			BLOB
		) WITHOUT ROWID;
		CREATE UNIQUE INDEX IF NOT EXISTS i1 ON entries (parent_id, id);
		CREATE UNIQUE INDEX IF NOT EXISTS i2 ON entries (parent_id, name);

		CREATE TABLE IF NOT EXISTS blocks_digests (
			entry_id		BLOB,
			block_no		INTEGER,
			digest			BLOB
		) /*WITHOUT ROWID*/;
		CREATE INDEX IF NOT EXISTS i3 ON blocks_digests (entry_id);
		CREATE UNIQUE INDEX IF NOT EXISTS i4 ON blocks_digests (entry_id, block_no);
		)EOS");

		ddl_sql.execute_all();
	}

	// get a prepared parsed and ready statment
	sqlite3pp::query st_sel_id(db, R"EOS(
		SELECT
			id
		FROM
			entries
		WHERE
			parent_id = :parent_id
			AND name = :name
	)EOS");

	sqlite3pp::query st_sel_pid(db, R"EOS(
		SELECT
			parent_id
		FROM
			entries
		WHERE
			id = :id
	)EOS");
	
	sqlite3pp::command st_ins(db, R"EOS(
		INSERT INTO entries (
			id, parent_id, name, is_dir, mtime, mtime_ns, digest
		) VALUES (:id, :parent_id, :name, :is_dir, :mtime, :mtime_ns, :digest)
	)EOS");
	
	sqlite3pp::command st_upd(db, R"EOS(
		UPDATE entries SET
			parent_id = :parent_id, name = :name,
			is_dir = :is_dir, mtime = :mtime, mtime_ns = :mtime_ns,
			digest = :digest
		WHERE
			id = :id
	)EOS");
	
	sqlite3pp::command st_blk_ins(db, R"EOS(
		INSERT INTO blocks_digests (
			entry_id, block_no, digest
		) VALUES (:entry_id, :block_no, :digest)
	)EOS");
	
	sqlite3pp::command st_blk_upd(db, R"EOS(
		UPDATE blocks_digests SET
			block_no = :block_no, digest = :digest
		WHERE
			entry_id = :entry_id
	)EOS");

	modified_only = modified_only;

	typedef std::vector<uint8_t> blob;
	
	std::unordered_map<std::string, blob> parents;
	blob id, parent_id, digest, block_digest;

	uint64_t blk_no;

	auto update_block_digest = [&] {
		// first if needed bind values to it
		st_blk_ins.bind(":id", id, sqlite3pp::nocopy);
		st_blk_ins.bind(":blk_no", blk_no);
		st_blk_ins.bind(":block_digest", block_digest, sqlite3pp::nocopy);

		try {
			sqlite3pp::transaction xct(db);
			st_blk_ins.execute();
		}
		catch( const sqlite3pp::database_error & e ) {
			std::cerr << db.error_code() << ", " << db.error_msg() << std::endl;
		}
	};

    directory_reader dr;
    dr.recursive = dr.list_directories = true;
	
	auto update_entry = [&] (
		const std::string & path_name,
		const std::string & name,
		bool is_dir,
		uint64_t mtime = 0,
		uint32_t mtime_nsec = 0)
	{
		auto bind = [&] (auto & st) {
			// first if needed bind values to it
			st.bind(":id", id, sqlite3pp::nocopy);
			st.bind(":name", name, sqlite3pp::nocopy);
			st.bind(":mtime", mtime);
			st.bind(":mtime_nsec", mtime_nsec);

			if( parent_id.empty() )
				st.bind(":parent_id", nullptr);
			else
				st.bind(":parent_id", parent_id, sqlite3pp::nocopy);

			if( digest.empty() )
				st.bind(":digest", nullptr);
			else
				st.bind(":digest", digest, sqlite3pp::nocopy);

			if( is_dir )
				st.bind(":is_dir", is_dir);
			else
				st.bind(":is_dir", nullptr);
		};
		
		for(;;) {
		
			bind(st_ins);

			try {
				sqlite3pp::transaction xct(db);
				// there is a convinience operator to execute and reset in one go
				st_ins.execute();

				if( dr.is_dir )
					parents.emplace(std::make_pair(path_name, id));
				break;
			}
			catch( const sqlite3pp::database_error & e) {
				std::cerr << e.err_code() << ", " << e.what() << std::endl;
				if( e.ex_err_code() != SQLITE_CONSTRAINT_PRIMARYKEY )
					throw e;

				st_sel_pid.bind(":id", id, sqlite3pp::nocopy);

				auto i = st_sel_pid.begin();

				if( (i->column_isnull("parent_id") && parent_id.empty())
					|| i->get<blob>("parent_id") == parent_id ) {
					bind(st_upd);
					st_upd.execute();
				}
				else {
					cdc512 ctx(id.cbegin(), id.cend());
					id.assign(std::cbegin(ctx.digest), std::cend(ctx.digest));
				}
			}
		}
	};
	
	dr.manipulator = [&] {
		cdc512 ctx(leave_uninitialized);
		{
			auto pit = parents.find(dr.path);

			if( pit == parents.cend() ) {

				if( dr.level > 1 ) {
					//ctx.update(id, dr.path.cbegin(), dr.path.cend());
					//st_sel_id.bind(":id", id, sqlite3pp::nocopy);
					//st_sel_id.bind(":name", dr.name, sqlite3pp::nocopy);
					throw std::runtime_error("Undefined behavior");
				}
				else {
					st_sel_id.bind(":id", nullptr);
					st_sel_id.bind(":name", dr.path, sqlite3pp::nocopy);
				}
				
				for( auto i : st_sel_id )
					i.get<blob>(parent_id, "id");

				if( st_sel_id.empty() ) {
					ctx.update(id, dr.path.cbegin(), dr.path.cend());
					parent_id.clear();
					digest.clear();
					
					update_entry(dr.path, dr.path, true);
					
					parent_id = std::move(id);
				}
				else {
					parents.emplace(std::make_pair(dr.path, parent_id));
				}
			}
			else {
				parent_id = pit->second;
			}

			ctx.update(id, dr.path_name.cbegin(), dr.path_name.cend());
		}
		
		std::ifstream in(dr.path_name, std::ios::binary);
		in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		
		ctx.init();
		blk_no = 0;

		while( !in.eof() ) {
			blk_no++;

			uint8_t buf[BLOCK_SIZE];
			in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE);
			auto r = in.gcount();
			std::memset(buf + r, 0, BLOCK_SIZE - r);

			cdc512 blk_ctx(std::cbegin(buf), std::cend(buf));
			block_digest.assign(blk_ctx.cbegin(), blk_ctx.cend());
			
			update_block_digest();

			ctx.update(std::cbegin(buf), std::cend(buf));
		}
		//try {
		//}
		//catch (std::ios_base::failure &fail) {
			// handle exception here
		//}

		ctx.finish();
		digest.assign(std::cbegin(ctx.digest), std::cend(ctx.digest));

		update_entry(dr.path_name, dr.name, dr.is_dir, dr.mtime, dr.mtime_nsec);
	};
	
    dr.read(get_cwd());
		
	// delete lost records
	db.execute(R"EOS(
		DELETE FROM blocks_digests
		WHERE
			entry_id IN (
				SELECT
					a.entry_id
				FROM
					blocks_digests AS a
					LEFT JOIN entries AS b
					ON a.entry_id = b.id
				WHERE
					b.id IS NULL
			)
	)EOS");
}
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
