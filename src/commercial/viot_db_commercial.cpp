#include "viot_db_commercial.h"
#include "viot_sqlite_wrapper.h"
#include "viot_config_commercial.h"

namespace viot {
namespace db {

viotDbCommercial::viotDbCommercial() {
    // 构造函数实现
}

viotDbCommercial::~viotDbCommercial() {
    // 析构函数实现
}
int viotDbCommercial::setNtripExpiresAt(const std::string &NtripExpiresAt){
    return viot::utils::SqliteWrapper::GetInstance().WriteValue(DB_LABEL_NTRIP_EXPIRES_TIME, NtripExpiresAt);
}
 int viotDbCommercial::getNtripExpiresAt(std::string &NtripExpiresAt){
    return viot::utils::SqliteWrapper::GetInstance().ReadValue(DB_LABEL_NTRIP_EXPIRES_TIME, NtripExpiresAt);
 }
int viotDbCommercial::setNtripInfo(const std::string &NtripInfo) {
   return viot::utils::SqliteWrapper::GetInstance().WriteValue(DB_LABEL_NTRIP_INFO, NtripInfo);
}

int viotDbCommercial::getNtripInfo(std::string &NtripInfo) {
   return viot::utils::SqliteWrapper::GetInstance().ReadValue(DB_LABEL_NTRIP_INFO, NtripInfo);
}

} // namespace db
} // namespace viot
