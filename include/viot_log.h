#pragma once
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace viot {
namespace utils {

// ANSI颜色代码
namespace Color {
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* BLACK   = "\033[30m";
    constexpr const char* RED     = "\033[31m";
    constexpr const char* GREEN   = "\033[32m";
    constexpr const char* YELLOW  = "\033[33m";
    constexpr const char* BLUE    = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN    = "\033[36m";
    constexpr const char* WHITE   = "\033[37m";
    
    // 高亮/粗体颜色
    constexpr const char* BRIGHT_BLACK   = "\033[90m";
    constexpr const char* BRIGHT_RED     = "\033[91m";
    constexpr const char* BRIGHT_GREEN   = "\033[92m";
    constexpr const char* BRIGHT_YELLOW  = "\033[93m";
    constexpr const char* BRIGHT_BLUE    = "\033[94m";
    constexpr const char* BRIGHT_MAGENTA = "\033[95m";
    constexpr const char* BRIGHT_CYAN    = "\033[96m";
    constexpr const char* BRIGHT_WHITE   = "\033[97m";
}

// 日志级别
enum class LogLevel {
    DEBUG,   // 青色
    INFO,    // 绿色
    WARN,    // 黄色
    ERROR    // 红色
};

// 获取颜色代码
inline const char* getColorCode(LogLevel level) {
    switch(level) {
        case LogLevel::DEBUG: return Color::CYAN;
        case LogLevel::INFO:  return Color::GREEN;
        case LogLevel::WARN:  return Color::YELLOW;
        case LogLevel::ERROR: return Color::RED;
        default: return Color::RESET;
    }
}

// 获取级别名称
inline const char* getLevelName(LogLevel level) {
    switch(level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// C++风格日志打印函数（带颜色和级别）
template<typename... Args>
void VIOT_PrintSaveLOG_Internal(LogLevel level, const char* function, int line, const std::string& format, Args... args) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* pTime = localtime(&tv.tv_sec);
    
    std::ostringstream oss;
    oss << pTime->tm_hour << ":" 
        << pTime->tm_min << ":" 
        << pTime->tm_sec << ":" 
        << tv.tv_usec / 1000;
    
    const char* color = getColorCode(level);
    
    std::cout << color << " " << oss.str() << " " 
              << syscall(__NR_gettid) << " "
              << "[" << getLevelName(level) << "] "
              << function << ":" << line << ": ";
    
    // 使用snprintf格式化字符串
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), format.c_str(), args...);
    std::cout << buffer << Color::RESET << std::endl;
    std::cout.flush();
}

// 重载版本：直接接受string（不需要格式化）
inline void VIOT_PrintSaveLOG_Internal(LogLevel level, const char* function, int line, const std::string& message) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* pTime = localtime(&tv.tv_sec);
    
    std::ostringstream oss;
    oss << pTime->tm_hour << ":" 
        << pTime->tm_min << ":" 
        << pTime->tm_sec << ":" 
        << tv.tv_usec / 1000;
    
    const char* color = getColorCode(level);
    
    std::cout << color << " " << oss.str() << " " 
              << syscall(__NR_gettid) << " "
              << "[" << getLevelName(level) << "] "
              << function << ":" << line << ": "
              << message << Color::RESET << std::endl;
    std::cout.flush();
}

// 支持自定义颜色的版本
template<typename... Args>
void VIOT_PrintSaveLOG_Color(const char* color_code, const char* function, int line, const std::string& format, Args... args) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* pTime = localtime(&tv.tv_sec);
    
    std::ostringstream oss;
    oss << pTime->tm_hour << ":" 
        << pTime->tm_min << ":" 
        << pTime->tm_sec << ":" 
        << tv.tv_usec / 1000;
    
    std::cout << color_code << " " << oss.str() << " " 
              << syscall(__NR_gettid) << " "
              << function << ":" << line << ": ";
    
    // 使用snprintf格式化字符串
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), format.c_str(), args...);
    std::cout << buffer << Color::RESET << std::endl;
    std::cout.flush();
}

// 重载版本：自定义颜色 + string
inline void VIOT_PrintSaveLOG_Color(const char* color_code, const char* function, int line, const std::string& message) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* pTime = localtime(&tv.tv_sec);
    
    std::ostringstream oss;
    oss << pTime->tm_hour << ":" 
        << pTime->tm_min << ":" 
        << pTime->tm_sec << ":" 
        << tv.tv_usec / 1000;
    
    std::cout << color_code << " " << oss.str() << " " 
              << syscall(__NR_gettid) << " "
              << function << ":" << line << ": "
              << message << Color::RESET << std::endl;
    std::cout.flush();
}

} // namespace utils
} // namespace viot

// 宏定义，自动传入函数名和行号
#define VIOT_LOG_DEBUG(...) viot::utils::VIOT_PrintSaveLOG_Internal(viot::utils::LogLevel::DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__)
#define VIOT_LOG_INFO(...)  viot::utils::VIOT_PrintSaveLOG_Internal(viot::utils::LogLevel::INFO, __FUNCTION__, __LINE__, __VA_ARGS__)
#define VIOT_LOG_WARN(...)  viot::utils::VIOT_PrintSaveLOG_Internal(viot::utils::LogLevel::WARN, __FUNCTION__, __LINE__, __VA_ARGS__)
#define VIOT_LOG_ERROR(...) viot::utils::VIOT_PrintSaveLOG_Internal(viot::utils::LogLevel::ERROR, __FUNCTION__, __LINE__, __VA_ARGS__)

// 自定义颜色宏 - 在参数中直接指定颜色
#define VIOT_LOG_COLOR(color, ...) viot::utils::VIOT_PrintSaveLOG_Color(color, __FUNCTION__, __LINE__, __VA_ARGS__)

// 保持向后兼容，默认使用INFO级别
#define VIOT_PrintSaveLOG(...) VIOT_LOG_INFO(__VA_ARGS__)
