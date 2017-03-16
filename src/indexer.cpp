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
#include <sys/stat.h>
#include <fcntl.h>
#include <share.h>
#ifndef _WIN32
#include <dirent.h>
#endif
#include <ctime>
#include <cerrno>
#include <cstring>
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
#if !defined(S_ILNK)
#define S_ILNK (fmt) false
#endif
//------------------------------------------------------------------------------
#include "config.h"
#include "scope_exit.hpp"
#include "std_ext.hpp"
#include "locale_traits.hpp"
#include "cdc512.hpp"
#include "indexer.hpp"
//------------------------------------------------------------------------------
namespace spacenet {
//------------------------------------------------------------------------------
const string::value_type path_delimiter[] =
#if _WIN32
    CPPX_U("\\")
#else
    CPPX_U("/")
#endif
;
//------------------------------------------------------------------------------
string temp_path(bool no_back_slash)
{
    string s;
#if _WIN32
    DWORD a = GetTempPathW(0, NULL);

    s.resize(a - 1);

    GetTempPathW(a, &s[0]);
#else
    s = P_tmpdir;
#endif
    if( no_back_slash ) {
        if( s.back() == path_delimiter[0] )
            s.pop_back();
    }
    else if( s.back() != path_delimiter[0] )
        s.push_back(path_delimiter[0]);

    return s;
}
//------------------------------------------------------------------------------
string temp_name(string dir, string pfx)
{
    constexpr int MAXTRIES = 10000;
	static std::atomic_int index;
	int pid = getpid();
    string s;

	if( dir.empty() )
        dir = temp_path(true);
    if( pfx.empty() )
        pfx = CPPX_U("temp");

#if _WIN32
    if( _waccess(dir.c_str(), R_OK | W_OK | X_OK) != 0 )
#else
    if( access(dir.c_str(), R_OK | W_OK | X_OK) != 0 )
#endif
        throw std::runtime_error("access denied to directory: " + str2utf(dir));

    size_t l = dir.size() + 1 + pfx.size() + 4 * (sizeof(int) * 3 + 2) + 1;
	s.resize(l);

	int try_n = 0;
	
	do {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
        int m = int(ts.tv_sec ^ uintptr_t(&s[0]) ^ uintptr_t(&s));
        int n = int(ts.tv_nsec ^ uintptr_t(&s[0]) ^ uintptr_t(&s));
#if _WIN32
        snwprintf(&s[0], l, CPPX_U("%ls\\%ls-%d-%d-%x-%x"),
#else
        snprintf(&s[0], l, CPPX_U("%s/%s-%d-%d-%x-%x"),
#endif
        dir.c_str(), pfx.c_str(), pid, index.fetch_add(1), m, n);
    }
#if _WIN32
    while( !_waccess(s.c_str(), F_OK) && try_n++ < MAXTRIES );
#else
    while( !access(s.c_str(), F_OK) && try_n++ < MAXTRIES );
#endif

	if( try_n >= MAXTRIES )
        throw std::range_error("function temp_name MAXTRIES reached");

#if _WIN32
    s.resize(wcslen(s.c_str()));
#else
    s.resize(strlen(s.c_str()));
#endif

	return s;
}
//------------------------------------------------------------------------------
string get_cwd(bool no_back_slash)
{
#if _WIN32
    DWORD a = GetCurrentDirectoryW(0, NULL);

    string s;
    s.resize(a - 1);

    GetCurrentDirectoryW(a, &s[0]);
#else
    string s;

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
        s += CPPX_U("/");

	s.shrink_to_fit();
#endif
    if( no_back_slash ) {
        if( s.back() == path_delimiter[0] )
            s.pop_back();
    }
    else if( s.back() != path_delimiter[0] )
        s.push_back(path_delimiter[0]);

    return s;
}
//------------------------------------------------------------------------------
string path2rel(const string & path, bool no_back_slash)
{
    string file_path = path;

    if( !file_path.empty() ) {
#if _WIN32
        file_path = std::str_replace<string>(file_path, CPPX_U("/"), CPPX_U("\\"));
#else
        file_path = str_replace(file_path, CPPX_U("\\"), CPPX_U("/"));
#endif
        //file_path = path.find(path_delimiter[0]) == 0 ? path.substr(1) : path;

        if( no_back_slash ) {
            if( file_path.back() == path_delimiter[0] )
                file_path.pop_back();
        }
        else if( file_path.back() != path_delimiter[0] )
            file_path.push_back(path_delimiter[0]);
    }
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
    file_stat(const string & file_name) noexcept {
		stat(file_name);
	}

    int stat(const string & file_name) noexcept {
#if _WIN32
        return _wstat64(file_name.c_str(), this);
#else
		return ::stat(file_name.c_str(), this);
#endif
	}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void directory_reader::read(const string & root_path)
{
    regex mask_regex(mask.empty() ? CPPX_U(".*") : mask);
    regex exclude_regex(exclude);

	path = root_path;

	if( path.back() == path_delimiter[0] )
		path.pop_back();

	struct stack_entry {
#if _WIN32
		HANDLE handle;
#else
		DIR * handle;
#endif
        string path;
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
    auto unpack_FILETIME = [] (FILETIME & ft, uint32_t & nsec) {
        ULARGE_INTEGER q;
        q.LowPart = ft.dwLowDateTime;
        q.HighPart = ft.dwHighDateTime;
        nsec = uint32_t((q.QuadPart % 10000000) * 100); // FILETIME is in units of 100 nsec.
        constexpr const uint64_t secs_between_epochs = 11644473600; // Seconds between 1.1.1601 and 1.1.1970 */
        return time_t((q.QuadPart / 10000000) - secs_between_epochs);
    };

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
            handle = FindFirstFileW((path + L"\\*").c_str(), &fdw);
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
            throw std::runtime_error(
                str2utf(L"Failed to open directory: " + path + L", " + std::to_wstring(err)));

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

            name = fdw.cFileName;
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

            atime = unpack_FILETIME(fdw.ftLastAccessTime, atime_ns);
            ctime = unpack_FILETIME(fdw.ftCreationTime, ctime_ns);
            mtime = unpack_FILETIME(fdw.ftLastWriteTime, mtime_ns);
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
            atime_ns = fs.st_atim.tv_nsec;
            ctime_ns = fs.st_ctim.tv_nsec;
            mtime_ns = fs.st_mtim.tv_nsec;
#else
            atime_ns = fs.st_atimensec;
            ctime_ns = fs.st_ctimensec;
            mtime_ns = fs.st_mtimensec;
#endif
			fsize = fs.st_size;
            is_reg = S_ISREG(fs.st_mode);
            is_dir = S_ISDIR(fs.st_mode);
            is_lnk = S_ISLNK(fs.st_mode);
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
            throw std::runtime_error(
                str2utf(L"Failed to read directory: " + path + L", " + std::to_wstring(err)));
		}
#endif
	}
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void directory_indexer::reindex(sqlite3pp::database & db)
{
    constexpr const size_t BLOCK_SIZE = 4096;

    db.execute_all(R"EOS(
        CREATE TABLE IF NOT EXISTS entries (
            is_alive		INTEGER NOT NULL,   /* boolean */
            /*id			BLOB PRIMARY KEY ON CONFLICT ABORT,*/
            parent_id		INTEGER NOT NULL,   /* link on entries rowid */
            name			TEXT NOT NULL,      /* file name*/
            is_dir			INTEGER,            /* boolean */
            mtime			INTEGER,            /* INTEGER as Unix Time, the number of seconds since 1970-01-01 00:00:00 UTC. */
            mtime_ns		INTEGER,            /* nanoseconds, if supported else zero or NULL */
            file_size		INTEGER,            /* file size in bytes */
            block_size		INTEGER,            /* file block size in bytes */
            digest			BLOB,               /* file checksum */
            UNIQUE(parent_id, name) ON CONFLICT ABORT
        ) /*WITHOUT ROWID*/;
        CREATE UNIQUE INDEX IF NOT EXISTS i1 ON entries (parent_id, name);
        CREATE INDEX IF NOT EXISTS i2 ON entries (is_alive);

        CREATE TABLE IF NOT EXISTS blocks_digests (
            entry_id		INTEGER NOT NULL,   /* link on entries rowid */
            block_no		INTEGER NOT NULL,   /* file block number starting from one */
            digest			BLOB,               /* file block checksum */
            UNIQUE(entry_id, block_no) ON CONFLICT ABORT
        ) /*WITHOUT ROWID*/;
        CREATE UNIQUE INDEX IF NOT EXISTS i3 ON blocks_digests (entry_id, block_no);
    )EOS");

    sqlite3pp::query st_sel(db, R"EOS(
        SELECT
            rowid,
            mtime,
            mtime_ns
        FROM
            entries
        WHERE
            parent_id = :parent_id
            AND name = :name
    )EOS");

	sqlite3pp::command st_ins(db, R"EOS(
        INSERT INTO entries (
            is_alive, parent_id, name, is_dir, mtime, mtime_ns, file_size, block_size, digest
        ) VALUES (1, :parent_id, :name, :is_dir, :mtime, :mtime_ns, :file_size, :block_size, NULL)
	)EOS");
	
	sqlite3pp::command st_upd(db, R"EOS(
        UPDATE entries SET
            is_alive = 1,
            parent_id = :parent_id,
            name = :name,
            is_dir = :is_dir,
            mtime = :mtime,
            mtime_ns = :mtime_ns,
            file_size = :file_size,
            block_size = :block_size,
            digest = NULL
		WHERE
            parent_id = :parent_id
            AND name = :name
	)EOS");

    sqlite3pp::command st_upd_touch(db, R"EOS(
        UPDATE entries SET
            is_alive = 1
        WHERE
            rowid = :id
    )EOS");

    sqlite3pp::command st_upd_digest(db, R"EOS(
        UPDATE entries SET
            is_alive = 1,
            digest = :digest
        WHERE
            rowid = :id
    )EOS");

	sqlite3pp::command st_blk_ins(db, R"EOS(
        INSERT INTO blocks_digests (
            entry_id, block_no, digest
        ) VALUES (
            :entry_id, :block_no, :digest)
	)EOS");

	sqlite3pp::command st_blk_upd(db, R"EOS(
		UPDATE blocks_digests SET
            digest = :digest
		WHERE
            entry_id = :entry_id
            AND block_no = :block_no
	)EOS");

    sqlite3pp::command st_blk_del(db, R"EOS(
        DELETE FROM blocks_digests
        WHERE
            entry_id = :entry_id
            AND block_no > :block_no
    )EOS");

    std::unordered_map<std::string, uint64_t> parents;

    typedef std::vector<uint8_t> blob;

    auto update_block_digest = [&] (
        uint64_t entry_id,
        uint64_t blk_no,
        const blob & block_digest)
    {
        auto bind = [&] (auto & st) {
            st.bind("entry_id", entry_id);
            st.bind("block_no", blk_no);
            st.bind("digest", block_digest, sqlite3pp::nocopy);
        };

        bind(st_blk_ins);

        auto exceptions_safe = db.exceptions();
        at_scope_exit( db.exceptions(exceptions_safe) );
        db.exceptions(false);

        if( st_blk_ins.execute() == SQLITE_CONSTRAINT_UNIQUE ) {
            db.exceptions(true);
            bind(st_blk_upd);
            st_blk_upd.execute();
        }
    };

	auto update_entry = [&] (
        uint64_t parent_id,
		const std::string & name,
		bool is_dir,
        uint64_t mtime,
        uint32_t mtime_ns,
        uint64_t file_size,
        uint64_t block_size,
        uint64_t * p_mtim = nullptr)
	{
		auto bind = [&] (auto & st) {
            st.bind("parent_id", parent_id);
            st.bind("name", name, sqlite3pp::nocopy);

            if( mtime == 0 )
                st.bind("mtime", nullptr);
            else
                st.bind("mtime", mtime);

            if( mtime_ns == 0 )
                st.bind("mtime_ns", nullptr);
            else
                st.bind("mtime_ns", mtime_ns);

			if( is_dir )
                st.bind("is_dir", is_dir);
			else
                st.bind("is_dir", nullptr);

            if( file_size == 0 )
                st.bind("file_size", nullptr);
            else
                st.bind("file_size", file_size);

            if( block_size == 0 )
                st.bind("block_size", nullptr);
            else
                st.bind("block_size", block_size);
        };
		
        auto exceptions_safe = db.exceptions();
        at_scope_exit( db.exceptions(exceptions_safe) );

        st_sel.bind("parent_id", parent_id);
        st_sel.bind("name", name, sqlite3pp::nocopy);

        uint64_t id = 0, mtim = 0, fmtim = mtime * 1000000000 + mtime_ns;

        auto get_id_mtim = [&] {
            auto i = st_sel.begin();

            if( i ) {
                id = i->get<uint64_t>("rowid");
                mtim = i->get<uint64_t>("mtime") * 1000000000
                    + i->get<uint32_t>("mtime_ns");
            }
        };

        if( modified_only_ )
            get_id_mtim();

        // then mtime not changed, just touch entry
        if( modified_only_ && id != 0 && (mtim == fmtim || fmtim == 0) ) {
            st_upd_touch.bind("id", id);
            st_upd_touch.execute();
        }
        else {
            db.exceptions(false);
            bind(st_ins);

            if( st_ins.execute() == SQLITE_CONSTRAINT_UNIQUE ) {
                db.exceptions(true);
                bind(st_upd);
                st_upd.execute();
            }
        }

        if( id == 0 )
            get_id_mtim();

        if( p_mtim != nullptr )
            *p_mtim = mtim;

        return id;
    };

    auto update_blocks = [&] (
        cdc512 & file_ctx,
        const string & path_name,
        const std::string & utf_path_name,
        uint64_t entry_id)
    {
        int in = -1;

        at_scope_exit(
            if( in != -1 )
                _close(in);
        );

        errno_t err =
#if _WIN32
        _wsopen_s
#else
        _sopen_s
#endif
        (&in, path_name.c_str(), _O_RDONLY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
        //std::wifstream in(dr.path_name, std::ios::binary);
        //in.exceptions(std::ios::failbit | std::ios::badbit);
        if( err != 0 )
            throw std::runtime_error(
                "Failed open file: " + utf_path_name + ", " + std::to_string(err));

        uint64_t blk_no = 0;

        for(;;) {//while( !in.eof() ) {
            blk_no++;

            uint8_t buf[BLOCK_SIZE];
            //in.read(reinterpret_cast<char *>(buf), BLOCK_SIZE);
            //auto r = in.gcount();

            auto r = _read(in, buf, BLOCK_SIZE);

            if( r == -1 ) {
                err = errno;
                throw std::runtime_error(
                    "Failed read file: " + utf_path_name + ", " + std::to_string(err));
            }

            if( r == 0 )
                break;

            if( size_t(r) < BLOCK_SIZE )
                std::memset(buf + r, 0, BLOCK_SIZE - r);

            blob block_digest;
            cdc512 blk_ctx(block_digest, std::cbegin(buf), std::cend(buf));
            update_block_digest(entry_id, blk_no, block_digest);

            st_blk_del.bind("entry_id", entry_id);
            st_blk_del.bind("block_no", blk_no);
            st_blk_del.execute();

            file_ctx.update(std::cbegin(buf), std::cend(buf));
        }
    };

    directory_reader dr;
    dr.recursive = dr.list_directories = true;
    dr.manipulator = [&] {
        // skip inaccessible files or directories
#if _WIN32
        if( _waccess(dr.path_name.c_str(), R_OK | (dr.is_dir ? X_OK : 0)) != 0 )
#else
        if( access(dr.path_name.c_str(), R_OK | (dr.is_dir ? X_OK : 0)) != 0 )
#endif
            return;

        auto utf_name = str2utf(dr.name);
        auto utf_path = str2utf(dr.path);
        auto utf_path_name = str2utf(dr.path_name);

        uint64_t parent_id = [&] {
            auto pit = parents.find(utf_path);

			if( pit == parents.cend() ) {
                if( dr.level > 1 )
					throw std::runtime_error("Undefined behavior");

                auto id = update_entry(0, utf_path, true, 0, 0, 0, 0);
                parents.emplace(std::make_pair(utf_path, id));
                return id;
            }

            return pit->second;
        }();

        uint64_t mtim, entry_id = update_entry(
            parent_id,
            utf_name,
            dr.is_dir,
            dr.mtime,
            dr.mtime_ns,
            dr.fsize,
            BLOCK_SIZE,
            &mtim);

        if( dr.is_dir )
            parents.emplace(std::make_pair(utf_path_name, entry_id));

        // if file modified then calculate digests
        if( dr.is_reg && mtim != dr.mtime * 1000000000 + dr.mtime_ns ) {
            cdc512 ctx;
            update_blocks(ctx, dr.path_name, utf_path_name, entry_id);

            blob digest;
            ctx.finish(digest);

            st_upd_digest.bind("id", entry_id);
            st_upd_digest.bind("digest", digest, sqlite3pp::nocopy);
            st_upd_digest.execute();
        }
	};
	
    dr.read(CPPX_U("C:\\hiew"));//get_cwd());
		
    auto cleanup_entries = [&db] {
        //sqlite3pp::query st_sel(db, R"EOS(
        //    SELECT
        //        rowid
        //    FROM
        //        entries
        //    WHERE
        //        is_alive = 0
        //)EOS");

        //sqlite3pp::command st_del(db, R"EOS(
        //    DELETE FROM blocks_digests
        //    WHERE
        //        entry_id = :entry_id
        //)EOS");

        sqlite3pp::transaction xct(db, true);

        //for( auto i = st_sel.begin(); i != st_sel.end(); i++ ) {
        //    st_del.bind(0, i->get<uint64_t>(0));
        //    st_del.execute();
        //}

        db.execute_all(R"EOS(
            DELETE FROM blocks_digests WHERE entry_id IN (
                SELECT
                    rowid
                FROM
                    entries
                WHERE
                    is_alive = 0
            );
            DELETE FROM entries WHERE is_alive = 0;
            UPDATE entries SET is_alive = 0;
        )EOS");

        xct.commit();
    };

    cleanup_entries();
}
//------------------------------------------------------------------------------
} // namespace spacenet
//------------------------------------------------------------------------------
