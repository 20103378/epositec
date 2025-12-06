#pragma once

// viot::utils::Singleton<T> 用于实现线程安全的单例模式。
// C++11及以上标准保证局部静态变量初始化线程安全。
// 用法：class MyClass : public Singleton<MyClass> { ... };

namespace robot {
namespace utils {

template <typename T>
class Singleton {
public:
    // 获取唯一实例。线程安全，首次调用时自动构造。
    static T& instance() {
        static T instance_;
        return instance_;
    }
    // GetInstance返回指针
    static T* GetInstance() { return &instance(); }
protected:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

} // namespace utils
} // namespace viot