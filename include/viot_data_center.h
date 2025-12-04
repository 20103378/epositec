#pragma once
#include <string>
#include <map>
#include <mutex>
#include <typeinfo>
#include <memory>
#include "utils/singleton.h"

namespace robot {
namespace utils {

// 数据中心，用于全局数据的存储和访问
class DataCenter : public robot::utils::Singleton<DataCenter>
{
public:
    DataCenter();
    ~DataCenter();

    // 设置键值对数据
    template<typename T>
    void set(const std::string& key, const T& value);

    // 获取键值对数据
    template<typename T>
    bool get(const std::string& key, T& value);

    // 检查键是否存在
    bool has(const std::string& key);

    // 删除指定键
    void remove(const std::string& key);

    // 清空所有数据
    void clear();

private:
    std::map<std::string, std::string> data_map_;
    std::mutex mutex_;
};

// 模板函数实现
template<typename T>
void DataCenter::set(const std::string& key, const T& value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 对于字符串类型直接存储
    if constexpr (std::is_same_v<T, std::string>) {
        data_map_[key] = value;
    }
    // 对于其他类型转换为字符串存储
    else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
        data_map_[key] = std::to_string(value);
    }
    else {
        // 其他类型需要用户自行实现序列化
        data_map_[key] = "";
    }
}

template<typename T>
bool DataCenter::get(const std::string& key, T& value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = data_map_.find(key);
    if (it == data_map_.end()) {
        return false;
    }

    // 对于字符串类型直接返回
    if constexpr (std::is_same_v<T, std::string>) {
        value = it->second;
        return true;
    }
    // 对于整数类型
    else if constexpr (std::is_integral_v<T>) {
        try {
            if constexpr (std::is_same_v<T, long> || std::is_same_v<T, long long>) {
                value = std::stoll(it->second);
            } else {
                value = std::stoi(it->second);
            }
            return true;
        } catch (...) {
            return false;
        }
    }
    // 对于浮点类型
    else if constexpr (std::is_floating_point_v<T>) {
        try {
            if constexpr (std::is_same_v<T, float>) {
                value = std::stof(it->second);
            } else {
                value = std::stod(it->second);
            }
            return true;
        } catch (...) {
            return false;
        }
    }
    
    return false;
}

} // namespace utils
} // namespace robot
