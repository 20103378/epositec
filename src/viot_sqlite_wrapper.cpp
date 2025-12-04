#include "viot_sqlite_wrapper.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <cstring>

namespace viot {
namespace utils {

SqliteWrapper::SqliteWrapper(const std::string &sqlite_file, const std::string &sqlite_table)
    : sqlite_file_(sqlite_file), sqlite_table_(sqlite_table), db_(nullptr) {
    // 打开数据库
    int rc = sqlite3_open(sqlite_file_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        db_ = nullptr;
        return;
    }

    // 创建表（如果不存在）
    std::stringstream ss;
    ss << "CREATE TABLE IF NOT EXISTS " << sqlite_table_ 
       << " (label TEXT PRIMARY KEY, value TEXT);";
    
    char *errMsg = nullptr;
    rc = sqlite3_exec(db_, ss.str().c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

SqliteWrapper::~SqliteWrapper() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

int SqliteWrapper::ReadValue(const std::string &label, std::string &buf) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!db_) {
        std::cerr << "Database is not open" << std::endl;
        return -1;
    }

    std::stringstream ss;
    ss << "SELECT value FROM " << sqlite_table_ << " WHERE label = ?;";
    
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, ss.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return -1;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, label.c_str(), -1, SQLITE_TRANSIENT);

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text) {
            buf = reinterpret_cast<const char*>(text);
        }
        sqlite3_finalize(stmt);
        return 0;
    } else if (rc == SQLITE_DONE) {
        // 没有找到记录
        sqlite3_finalize(stmt);
        return -2;
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }
}

int SqliteWrapper::WriteValue(const std::string &label, const std::string &buf) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!db_) {
        std::cerr << "Database is not open" << std::endl;
        return -1;
    }

    std::stringstream ss;
    ss << "INSERT OR REPLACE INTO " << sqlite_table_ 
       << " (label, value) VALUES (?, ?);";
    
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, ss.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return -1;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, label.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, buf.c_str(), -1, SQLITE_TRANSIENT);

    // 执行插入/更新
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
        return -1;
    }

    return 0;
}

} // namespace utils
} // namespace viot
