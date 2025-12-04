#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include "utils/singleton.h"

struct sqlite3;

#define VIOT_DB_FILE_PATH "viot.db"
namespace viot {
    namespace utils
    {
        class SqliteWrapper: public robot::utils::Singleton<SqliteWrapper>
        {
        public:
            SqliteWrapper(const std::string &sqlite_file  = VIOT_DB_FILE_PATH,const std::string &sqlite_table = "viotTable");
            ~SqliteWrapper();
            int ReadValue(const std::string &label, std::string &buf);
            int WriteValue(const std::string &label, const std::string &buf);
        private:
           std::string sqlite_file_;
           std::string sqlite_table_;
           sqlite3 *db_;
           std::mutex mutex_;
        };
    } // namespace utils
} // namespace viot
