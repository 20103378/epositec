#include "../../include/interface/viot_sqlite_wrapper.h"
#include <sqlite3.h>
#include <iostream>

namespace viot {
namespace db {

SQLiteManager::SQLiteManager() : db_(nullptr) {}

SQLiteManager::~SQLiteManager() {
    close();
}

bool SQLiteManager::open(const std::string& db_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (db_ != nullptr) {
        error_msg_ = "数据库已打开";
        return false;
    }

    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        error_msg_ = std::string("无法打开数据库: ") + sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    std::cout << "SQLite数据库打开成功: " << db_path << std::endl;
    return true;
}

void SQLiteManager::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
        std::cout << "SQLite数据库已关闭" << std::endl;
    }
}

bool SQLiteManager::isOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return db_ != nullptr;
}

bool SQLiteManager::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (db_ == nullptr) {
        error_msg_ = "数据库未打开";
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        error_msg_ = std::string("SQL执行错误: ") + (err_msg ? err_msg : "未知错误");
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

int SQLiteManager::callbackWrapper(void* data, int argc, char** argv, char** col_names) {
    auto* callback = static_cast<std::function<void(int, char**, char**)>*>(data);
    if (callback) {
        (*callback)(argc, argv, col_names);
    }
    return 0;
}

bool SQLiteManager::query(const std::string& sql, 
                          std::function<void(int, char**, char**)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (db_ == nullptr) {
        error_msg_ = "数据库未打开";
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), callbackWrapper, &callback, &err_msg);
    
    if (rc != SQLITE_OK) {
        error_msg_ = std::string("SQL查询错误: ") + (err_msg ? err_msg : "未知错误");
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool SQLiteManager::beginTransaction() {
    return execute("BEGIN TRANSACTION;");
}

bool SQLiteManager::commit() {
    return execute("COMMIT;");
}

bool SQLiteManager::rollback() {
    return execute("ROLLBACK;");
}

int64_t SQLiteManager::getLastInsertId() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (db_ == nullptr) {
        return -1;
    }
    
    return sqlite3_last_insert_rowid(db_);
}

int SQLiteManager::getChanges() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (db_ == nullptr) {
        return 0;
    }
    
    return sqlite3_changes(db_);
}

std::string SQLiteManager::getErrorMsg() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return error_msg_;
}

} // namespace db
} // namespace viot
