#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>

// 前向声明 sqlite3
struct sqlite3;
struct sqlite3_stmt;

namespace viot {
namespace db {

class SQLiteManager {
public:
    SQLiteManager();
    ~SQLiteManager();

    // 禁止拷贝
    SQLiteManager(const SQLiteManager&) = delete;
    SQLiteManager& operator=(const SQLiteManager&) = delete;

    // 打开/关闭数据库
    bool open(const std::string& db_path);
    void close();
    bool isOpen() const;

    // 执行SQL语句（无返回结果）
    bool execute(const std::string& sql);

    // 查询SQL语句（有返回结果）
    // callback: 每行数据回调函数，参数为列数、列值数组、列名数组
    bool query(const std::string& sql, 
               std::function<void(int, char**, char**)> callback);

    // 开启/提交/回滚事务
    bool beginTransaction();
    bool commit();
    bool rollback();

    // 获取最后插入的行ID
    int64_t getLastInsertId() const;

    // 获取受影响的行数
    int getChanges() const;

    // 获取错误信息
    std::string getErrorMsg() const;

private:
    sqlite3* db_;
    mutable std::mutex mutex_;
    std::string error_msg_;

    static int callbackWrapper(void* data, int argc, char** argv, char** col_names);
};

} // namespace db
} // namespace viot
