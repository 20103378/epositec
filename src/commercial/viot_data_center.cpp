#include "viot_data_center.h"
#include "viot_log.h"

namespace robot {
namespace utils {

DataCenter::DataCenter()
{
    VIOT_LOG_DEBUG("DataCenter构造函数");
}

DataCenter::~DataCenter()
{
    std::lock_guard<std::mutex> lock(mutex_);
    data_map_.clear();
    VIOT_LOG_DEBUG("DataCenter析构函数");
}

bool DataCenter::has(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return data_map_.find(key) != data_map_.end();
}

void DataCenter::remove(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = data_map_.find(key);
    if (it != data_map_.end()) {
        data_map_.erase(it);
        VIOT_LOG_DEBUG("DataCenter删除键: %s", key.c_str());
    }
}

void DataCenter::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    data_map_.clear();
    VIOT_LOG_INFO("DataCenter清空所有数据");
}

} // namespace utils
} // namespace robot
