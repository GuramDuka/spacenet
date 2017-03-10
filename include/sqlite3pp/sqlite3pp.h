// sqlite3pp.h
//
// The MIT License
//
// Copyright (c) 2015 Wongoo Lee (iwongu at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef SQLITE3PP_H
#define SQLITE3PP_H

#define SQLITE3PP_VERSION "1.0.6"
#define SQLITE3PP_VERSION_MAJOR 1
#define SQLITE3PP_VERSION_MINOR 0
#define SQLITE3PP_VERSION_PATCH 6

#include <cstring>
#include <functional>
#include <iterator>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <tuple>
//#include <variant>
#include <vector>
#include <unordered_map>

namespace sqlite3pp {
    namespace ext {
        class function;
        class aggregate;
    }

    template <typename T>
    struct convert {
        using to_int = int;
    };

    class noncopyable {
    protected:
        noncopyable() = default;
        ~noncopyable() = default;

        noncopyable(noncopyable&&) = default;
        noncopyable& operator=(noncopyable&&) = default;

        noncopyable(noncopyable const&) = delete;
        noncopyable& operator=(noncopyable const&) = delete;
    };

    class database;

    class database_error : public std::runtime_error {
    public:
        explicit database_error(char const * msg, int err_code = 0) :
            std::runtime_error(msg), err_code_(err_code), ex_err_code_(0) {}
        explicit database_error(database & db);
        
        int & err_code() const {
            return err_code();
        }
        int & ex_err_code() const {
            return ex_err_code();
        }
    private:
        int err_code_;
        int ex_err_code_;
    };

    namespace {
        int busy_handler_impl(void* p, int cnt);
        int commit_hook_impl(void* p);
        void rollback_hook_impl(void* p);
        void update_hook_impl(void* p, int opcode, char const* dbname, char const* tablename, long long int rowid);
        int authorizer_impl(void* p, int evcode, char const* p1, char const* p2, char const* dbname, char const* tvname);
    } // namespace
    
    class database : noncopyable {
        friend class statement;
        friend class database_error;
        friend class ext::function;
        friend class ext::aggregate;
    public:
        using busy_handler = std::function<int (int) >;
        using commit_handler = std::function<int ()>;
        using rollback_handler = std::function<void ()>;
        using update_handler = std::function<void (int, char const*, char const*, long long int) >;
        using authorize_handler = std::function<int (int, char const*, char const*, char const*, char const*) >;
        using backup_handler = std::function<void (int, int, int) >;

        explicit database(char const* dbname = nullptr, int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, const char* vfs = nullptr)
        : db_(nullptr) {
            if (dbname) {
                auto rc = connect(dbname, flags, vfs);
                if (rc != SQLITE_OK)
                    throw database_error("can't connect database");
            }
        }

        explicit database(const std::string & dbname, int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, const char* vfs = nullptr)
            : database(dbname.c_str(), flags, vfs) {
        }

        database(database&& db) :
            db_(std::move(db.db_)),
            bh_(std::move(db.bh_)),
            ch_(std::move(db.ch_)),
            rh_(std::move(db.rh_)),
            uh_(std::move(db.uh_)),
            ah_(std::move(db.ah_))
        {
            db.db_ = nullptr;
        }

        database& operator=(database&& db) {
            db_ = std::move(db.db_);
            db.db_ = nullptr;

            bh_ = std::move(db.bh_);
            ch_ = std::move(db.ch_);
            rh_ = std::move(db.rh_);
            uh_ = std::move(db.uh_);
            ah_ = std::move(db.ah_);

            return *this;
        }

        ~database() {
            disconnect();
        }

        int connect(char const* dbname, int flags, char const* vfs) {
            disconnect();

            return sqlite3_open_v2(dbname, &db_, flags, vfs);
        }

        int disconnect() {
            auto rc = SQLITE_OK;
            if (db_) {
                rc = sqlite3_close_v2(db_);
                if (rc == SQLITE_OK) {
                    db_ = nullptr;
                }
            }

            return rc;
        }

        int attach(char const* dbname, char const* name) {
            return executef("ATTACH '%q' AS '%q'", dbname, name);
        }

        int detach(char const* name) {
            return executef("DETACH '%q'", name);
        }

        int backup(char const* dbname, database& destdb, char const* destdbname, backup_handler h, int step_page = 5) {
            sqlite3_backup* bkup = sqlite3_backup_init(destdb.db_, destdbname, db_, dbname);
            if (!bkup) {
                return error_code();
            }
            auto rc = SQLITE_OK;
            do {
                rc = sqlite3_backup_step(bkup, step_page);
                if (h) {
                    h(sqlite3_backup_remaining(bkup), sqlite3_backup_pagecount(bkup), rc);
                }
            } while (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
            sqlite3_backup_finish(bkup);
            return rc;
        }

        int backup(database& destdb, backup_handler h) {
            return backup("main", destdb, "main", h);
        }
        
        void set_busy_handler(busy_handler h) {
            bh_ = h;
            sqlite3_busy_handler(db_, bh_ ? busy_handler_impl : 0, &bh_);
        }

        void set_commit_handler(commit_handler h) {
            ch_ = h;
            sqlite3_commit_hook(db_, ch_ ? commit_hook_impl : 0, &ch_);
        }

        void set_rollback_handler(rollback_handler h) {
            rh_ = h;
            sqlite3_rollback_hook(db_, rh_ ? rollback_hook_impl : 0, &rh_);
        }

        void set_update_handler(update_handler h) {
            uh_ = h;
            sqlite3_update_hook(db_, uh_ ? update_hook_impl : 0, &uh_);
        }

        void set_authorize_handler(authorize_handler h) {
            ah_ = h;
            sqlite3_set_authorizer(db_, ah_ ? authorizer_impl : 0, &ah_);
        }

        long long int last_insert_rowid() const {
            return sqlite3_last_insert_rowid(db_);
        }

        int enable_foreign_keys(bool enable = true) {
            return sqlite3_db_config(db_, SQLITE_DBCONFIG_ENABLE_FKEY, enable ? 1 : 0, nullptr);
        }

        int enable_triggers(bool enable = true) {
            return sqlite3_db_config(db_, SQLITE_DBCONFIG_ENABLE_TRIGGER, enable ? 1 : 0, nullptr);
        }

        int enable_extended_result_codes(bool enable = true) {
            return sqlite3_extended_result_codes(db_, enable ? 1 : 0);
        }

        int changes() const {
            return sqlite3_changes(db_);
        }

        int error_code() const {
            return sqlite3_errcode(db_);
        }

        int extended_error_code() const {
            return sqlite3_extended_errcode(db_);
        }

        char const* error_msg() const {
            return sqlite3_errmsg(db_);
        }

        int execute(char const* sql) {
            return sqlite3_exec(db_, sql, 0, 0, 0);
        }

        int executef(char const* sql, ...) {
            va_list ap;
            va_start(ap, sql);
            std::shared_ptr<char> msql(sqlite3_vmprintf(sql, ap), sqlite3_free);
            va_end(ap);

            return execute(msql.get());
        }

        int set_busy_timeout(int ms) {
            return sqlite3_busy_timeout(db_, ms);
        }
                
    private:
        sqlite3* db_;

        busy_handler bh_;
        commit_handler ch_;
        rollback_handler rh_;
        update_handler uh_;
        authorize_handler ah_;
    };

    namespace {
        int busy_handler_impl(void* p, int cnt) {
            auto h = static_cast<database::busy_handler*> (p);
            return (*h)(cnt);
        }

        int commit_hook_impl(void* p) {
            auto h = static_cast<database::commit_handler*> (p);
            return (*h)();
        }

        void rollback_hook_impl(void* p) {
            auto h = static_cast<database::rollback_handler*> (p);
            (*h)();
        }

        void update_hook_impl(void* p, int opcode, char const* dbname, char const* tablename, long long int rowid) {
            auto h = static_cast<database::update_handler*> (p);
            (*h)(opcode, dbname, tablename, rowid);
        }

        int authorizer_impl(void* p, int evcode, char const* p1, char const* p2, char const* dbname, char const* tvname) {
            auto h = static_cast<database::authorize_handler*> (p);
            return (*h)(evcode, p1, p2, dbname, tvname);
        }

    } // namespace

    enum copy_semantic {
        copy, nocopy
    };

    class statement : noncopyable {
    public:
        int prepare(char const* stmt) {
            finish();
            auto rc = prepare_impl(stmt);
            if (rc != SQLITE_OK)
                throw database_error(db_);
            return rc;
        }

        int finish() {
            auto rc = SQLITE_OK;
            if (stmt_ != nullptr) {
                rc = finish_impl(stmt_);
                if (rc != SQLITE_OK)
                    throw database_error(db_);
                stmt_ = nullptr;
            }
            tail_ = nullptr;

            return rc;
        }

        bool empty() {
            return empty_;
        }
        
        int step() {
            int rc = sqlite3_step(stmt_);
            if( rc == SQLITE_ROW )
                empty_ = false;
            return rc;
        }

        int reset() {
            return sqlite3_reset(stmt_);
        }

        int bind(int idx, bool value) {
            return sqlite3_bind_int(stmt_, idx, value ? 1 : 0);
        }
        
        int bind(int idx, int value) {
            return sqlite3_bind_int(stmt_, idx, value);
        }

        int bind(int idx, unsigned value) {
            return sqlite3_bind_int(stmt_, idx, value);
        }
        
        int bind(int idx, double value) {
            return sqlite3_bind_double(stmt_, idx, value);
        }

        int bind(int idx, long long int value) {
            return sqlite3_bind_int64(stmt_, idx, value);
        }

        int bind(int idx, long long unsigned value) {
            return sqlite3_bind_int64(stmt_, idx, value);
        }

        int bind(int idx, uint64_t value) {
            return sqlite3_bind_int64(stmt_, idx, value);
        }
        
        int bind(int idx, char const* value, copy_semantic fcopy) {
            return sqlite3_bind_text(stmt_, idx, value, std::strlen(value), fcopy == copy ? SQLITE_TRANSIENT : SQLITE_STATIC);
        }

        int bind(int idx, void const* value, int n, copy_semantic fcopy) {
            return sqlite3_bind_blob(stmt_, idx, value, n, fcopy == copy ? SQLITE_TRANSIENT : SQLITE_STATIC);
        }

        int bind(int idx, std::string const& value, copy_semantic fcopy) {
            return sqlite3_bind_text(stmt_, idx, value.c_str(), value.size(), fcopy == copy ? SQLITE_TRANSIENT : SQLITE_STATIC);
        }

        int bind(int idx) {
            return sqlite3_bind_null(stmt_, idx);
        }

        int bind(int idx, std::nullptr_t) {
            return bind(idx);
        }

        int bind(char const* name, bool value) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value);
        }
        
        int bind(char const* name, int value) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value);
        }

        int bind(char const* name, unsigned value) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value);
        }
        
        int bind(char const* name, double value) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value);
        }

        int bind(char const* name, long long int value) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value);
        }

        int bind(char const* name, long long unsigned value) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value);
        }

        int bind(char const* name, uint64_t value) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value);
        }
        
        int bind(char const* name, char const* value, copy_semantic fcopy) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value, fcopy);
        }

        int bind(char const* name, void const* value, int n, copy_semantic fcopy) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value, n, fcopy);
        }

        int bind(char const* name, std::string const& value, copy_semantic fcopy) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx, value, fcopy);
        }

        int bind(char const* name) {
            auto idx = sqlite3_bind_parameter_index(stmt_, name);
            return bind(idx);
        }

        int bind(char const* name, std::nullptr_t) {
            return bind(name);
        }

        int bind(const std::string & name, std::nullptr_t) {
            return bind(name.c_str());
        }
        
        template <typename T>
        int bind(const std::string & name, const T & value) {
            return bind(name.c_str(), value);
        }

        template <typename T>
        int bind(const std::string & name, const T & value, copy_semantic fcopy) {
            return bind(name.c_str(), value, fcopy);
        }

        template <typename T>
        int bind(const char * name, const std::vector<T> & value, copy_semantic fcopy) {
            return bind(name, (const void *) &value[0], int(value.size() * sizeof(T)), fcopy);
        }
        
        template <typename T>
        int bind(const std::string & name, const std::vector<T> & value, copy_semantic fcopy) {
            return bind(name.c_str(), (const void *) &value[0], int(value.size() * sizeof(T)), fcopy);
        }

        template <typename T>
        int bind(const std::string & name, const T & value, int n, copy_semantic fcopy) {
            return bind(name.c_str(), value, n, fcopy);
        }
        
    protected:
        explicit statement(database& db, char const* stmt = nullptr)
            : db_(db), stmt_(0), tail_(0), empty_(true)
        {
            if (stmt) {
                auto rc = prepare(stmt);
                if (rc != SQLITE_OK)
                    throw database_error(db_);
            }
        }

        ~statement() {
            // finish() can return error. If you want to check the error, call
            // finish() explicitly before this object is destructed.
            finish();
        }

        int prepare_impl(char const* stmt) {
            return sqlite3_prepare_v2(db_.db_, stmt, std::strlen(stmt), &stmt_, &tail_);
        }

        int finish_impl(sqlite3_stmt* stmt) {
            return sqlite3_finalize(stmt);
        }

    protected:
        database& db_;
        sqlite3_stmt* stmt_;
        char const* tail_;
        bool empty_;
    };

    class command : public statement {
    public:
        class bindstream {
        public:
            bindstream(command& cmd, int idx) : cmd_(cmd), idx_(idx) {}

            template <class T>
            bindstream& operator<<(T value) {
                auto rc = cmd_.bind(idx_, value);
                if (rc != SQLITE_OK) {
                    throw database_error(cmd_.db_);
                }
                ++idx_;
                return *this;
            }

            bindstream& operator<<(char const* value) {
                auto rc = cmd_.bind(idx_, value, copy);
                if (rc != SQLITE_OK) {
                    throw database_error(cmd_.db_);
                }
                ++idx_;
                return *this;
            }

            bindstream& operator<<(std::string const& value) {
                auto rc = cmd_.bind(idx_, value, copy);
                if (rc != SQLITE_OK) {
                    throw database_error(cmd_.db_);
                }
                ++idx_;
                return *this;
            }

        private:
            command& cmd_;
            int idx_;
        };

        explicit command(database& db, char const* stmt = nullptr) : statement(db, stmt) {}
        explicit command(database& db, const std::string & stmt) : command(db, stmt.c_str()) {}

        bindstream binder(int idx = 1) {
            return bindstream(*this, idx);
        }

        int execute() {
            auto rc = step();
            if (rc == SQLITE_ROW)
                rc = step();
            if (rc == SQLITE_DONE)
                rc = SQLITE_OK;
            if (rc != SQLITE_OK)
                throw database_error(db_);

            return rc;
        }

        int execute_all() {
            auto rc = execute();

            char const* sql = tail_;

            while (std::strlen(sql) > 0) { // sqlite3_complete() is broken.
                sqlite3_stmt* old_stmt = stmt_;

                if ((rc = prepare_impl(sql)) != SQLITE_OK)
                    throw database_error(db_);

                // If the input text contains no SQL (if the input is an empty string or a comment) then stmt_ is set to nullptr
                if (stmt_ == nullptr) {
                    stmt_ = old_stmt;
                    break;
                }

                if ((rc = sqlite3_transfer_bindings(old_stmt, stmt_)) != SQLITE_OK) {
                    finish_impl(old_stmt);
                    throw database_error(db_);
                }

                finish_impl(old_stmt);

                if ((rc = execute()) != SQLITE_OK)
                    throw database_error(db_);

                sql = tail_;
            }

            return rc;
        }
    };

    enum query_column_type {
        SQL_INT = SQLITE_INTEGER,
        SQL_FLT = SQLITE_FLOAT,
        SQL_TXT = SQLITE_TEXT,
        SQL_BLB = SQLITE_BLOB,
        SQL_NIL = SQLITE_NULL
    };
    
    class query : public statement {
    public:
        class row {
        public:
            class getstream {
            public:
                getstream(row* rws, int idx) : rws_(rws), idx_(idx) {}

                template <typename T>
                getstream & operator >> (T& value) {
                    value = rws_->get(idx_, T());
                    ++idx_;
                    return *this;
                }
            private:
                row * rws_;
                int idx_;
            };

            explicit row(query * cmd) : cmd_(cmd) {}

            int data_count() const {
                return sqlite3_data_count(cmd_->stmt_);
            }
            
            query_column_type column_type(int idx) const {
                return static_cast<query_column_type>(sqlite3_column_type(cmd_->stmt_, idx));
            }

            query_column_type column_type(const char * name) const {
                return static_cast<query_column_type>(sqlite3_column_type(cmd_->stmt_, cmd_->name2idx(name)));
            }

            query_column_type column_type(const std::string & name) const {
                return static_cast<query_column_type>(sqlite3_column_type(cmd_->stmt_, cmd_->name2idx(name)));
            }
            
            bool column_isnull(int idx) const {
                return column_type(idx) == SQL_NIL;
            }
            
            bool column_isnull(const char * name) const {
                return column_type(name) == SQL_NIL;
            }

            bool column_isnull(const std::string & name) const {
                return column_type(name) == SQL_NIL;
            }
            
            int column_bytes(int idx) const {
                return sqlite3_column_bytes(cmd_->stmt_, idx);
            }

            template <typename T> T get(int idx) const {
                return get(idx, T());
            }

            template <typename T> T get(const char * name) const {
                return get(cmd_->name2idx(name), T());
            }

            template <typename T> T get(const std::string & name) const {
                return get(cmd_->name2idx(name), T());
            }

            template <typename T> T & get(T & v, int idx) const {
                return copy_impl(idx, v);
            }

            template <typename T> T & get(T & v, const char * name) const {
                return copy_impl(cmd_->name2idx(name), v);
            }

            template <typename T> T & get(T & v, const std::string & name) const {
                return copy_impl(cmd_->name2idx(name), v);
            }

            template <class... Ts>
            std::tuple<Ts...> get_columns(typename convert<Ts>::to_int... idxs) const {
                return std::make_tuple(get(idxs, Ts())...);
            }

            getstream getter(int idx) {
                return getstream(this, idx);
            }
            
            //using var_t = std::variant<int, long long int, double, std::string, std::vector<uint8_t>>;
        private:
            int get(int idx, int) const {
                return sqlite3_column_int(cmd_->stmt_, idx);
            }

            int & copy_impl(int idx, int & v) const {
                return v = sqlite3_column_int(cmd_->stmt_, idx);
            }

            double get(int idx, double) const {
                return sqlite3_column_double(cmd_->stmt_, idx);
            }

            double & copy_impl(int idx, double & v) const {
                return v = sqlite3_column_double(cmd_->stmt_, idx);
            }
            
            long long int get(int idx, long long int) const {
                return sqlite3_column_int64(cmd_->stmt_, idx);
            }

            long long int & copy_impl(int idx, long long int & v) const {
                return v = sqlite3_column_int64(cmd_->stmt_, idx);
            }
            
            char const* get(int idx, char const*) const {
                return reinterpret_cast<char const*> (sqlite3_column_text(cmd_->stmt_, idx));
            }

            std::string get(int idx, std::string) const {
                return reinterpret_cast<char const*> (sqlite3_column_text(cmd_->stmt_, idx));
            }

            std::string & copy_impl(int idx, std::string & v) const {
                return v = reinterpret_cast<char const*> (sqlite3_column_text(cmd_->stmt_, idx));
            }
            
            void const* get(int idx, void const*) const {
                return sqlite3_column_blob(cmd_->stmt_, idx);
            }

            template <typename T>
            std::vector<T> get(int idx, std::vector<T>) const {
                int s = sqlite3_column_bytes(cmd_->stmt_, idx);
                const T * p = static_cast<const T *> (sqlite3_column_blob(cmd_->stmt_, idx));
                return std::vector<T>(p, p + s / sizeof (T) - 1);
            }

            template <typename T>
            std::vector<T> & copy_impl(int idx, std::vector<T> & v) const {
                int s = sqlite3_column_bytes(cmd_->stmt_, idx);
                const T * p = static_cast<const T *> (sqlite3_column_blob(cmd_->stmt_, idx));
                v.assign(p, p + s / sizeof (T) - 1);
                return v;
            }
            
            std::nullptr_t get(int /*idx*/, std::nullptr_t) const {
                return nullptr;
            }
        protected:
            query * cmd_;
        };

        class query_iterator : public std::iterator<std::input_iterator_tag, row>, private row {
        public:
            query_iterator() : row(nullptr), rc_(SQLITE_DONE) {}

            explicit query_iterator(query * cmd) : row(cmd) {
                rc_ = cmd_->step();
                if (rc_ != SQLITE_ROW && rc_ != SQLITE_DONE)
                    throw database_error(cmd_->db_);
            }

            bool operator == (query_iterator const& other) const {
                return rc_ == other.rc_;
            }

            bool operator != (query_iterator const& other) const {
                return rc_ != other.rc_;
            }

            query_iterator & operator ++ () {
                rc_ = cmd_->step();
                if (rc_ != SQLITE_ROW && rc_ != SQLITE_DONE)
                    throw database_error(cmd_->db_);
                return *this;
            }
            
            value_type & operator * () const {
                return *const_cast<value_type *>(static_cast<const value_type *>(this));
            }

            value_type * operator -> () const {
                return const_cast<value_type *>(static_cast<const value_type *>(this));
            }
        private:
            int rc_;
        };

        explicit query(database& db, char const* stmt = nullptr) : statement(db, stmt) {}
        explicit query(database& db, const std::string & stmt) : query(db, stmt.c_str()) {}

        int column_count() const {
            return sqlite3_column_count(stmt_);
        }

        char const* column_name(int idx) const {
            return sqlite3_column_name(stmt_, idx);
        }

        char const* column_decltype(int idx) const {
            return sqlite3_column_decltype(stmt_, idx);
        }

        int name2idx(const char * name) const {
            auto i = get_cache().find(name);
            
            if( i == cache_.cend() )
                throw database_error("Invalid column name");

            return i->second;
        }

        int name2idx(const std::string & name) const {
            auto i = get_cache().find(name);
            
            if( i == cache_.cend() )
                throw database_error("Invalid column name");

            return i->second;
        }

        using iterator = query_iterator;

        iterator begin() {
            return query_iterator(this);
        }

        iterator end() {
            return query_iterator();
        }

    private:
        mutable std::unordered_map<std::string, int> cache_;
        
        decltype(cache_) & get_cache() const {
            if( cache_.empty() ) {
                int k = sqlite3_column_count(stmt_);
                
                for( int i = 0; i < k; i++ )
                    cache_.emplace(std::make_pair(sqlite3_column_name(stmt_, i), i));
            }
            
            return cache_;
        }
    };

    class transaction : noncopyable {
    public:

        explicit transaction(database& db, bool fcommit = false, bool freserve = false)
            : db_(&db), fcommit_(fcommit)
        {
            int rc = db_->execute(freserve ? "BEGIN IMMEDIATE" : "BEGIN");
            if (rc != SQLITE_OK)
                throw database_error(*db_);
        }

        ~transaction() {
            if (db_) {
                // execute() can return error. If you want to check the error,
                // call commit() or rollback() explicitly before this object is
                // destructed.
                db_->execute(fcommit_ ? "COMMIT" : "ROLLBACK");
            }
        }

        int commit() {
            auto db = db_;
            db_ = nullptr;
            int rc = db->execute("COMMIT");
            return rc;
        }

        int rollback() {
            auto db = db_;
            db_ = nullptr;
            int rc = db->execute("ROLLBACK");
            return rc;
        }

    private:
        database * db_;
        bool fcommit_;
    };

    inline database_error::database_error(database& db) :
        std::runtime_error(sqlite3_errmsg(db.db_)),
        err_code_(sqlite3_errcode(db.db_)),
        ex_err_code_(sqlite3_extended_errcode(db.db_))
    {
    }

} // namespace sqlite3pp

#endif
