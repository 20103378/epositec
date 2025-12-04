#include "viot_http_commercial.h"
#include "viot_log.h"
#include <curl/curl.h>
#include <cstring>
#include <sstream>

namespace viot {
namespace http {

// 静态成员初始化
std::mutex IOThttp::tokeMutex;

IOThttp::IOThttp() {
    cerBuff.size = 0;
    cerBuff.buff = nullptr;
    
    // 初始化curl全局环境
    curl_global_init(CURL_GLOBAL_ALL);
    
    VIOT_LOG_DEBUG("IOThttp构造函数");
}

IOThttp::~IOThttp() {
    if (cerBuff.buff) {
        delete[] cerBuff.buff;
        cerBuff.buff = nullptr;
    }
    cerBuff.size = 0;
    
    // 清理curl全局环境
    curl_global_cleanup();
    
    VIOT_LOG_DEBUG("IOThttp析构函数");
}

// HTTPS回调函数，用于接收响应数据
size_t IOThttp::parseHttpsInfoCallback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realSize = size * nmemb;
    std::string *response = static_cast<std::string*>(userdata);
    
    if (response && ptr) {
        response->append(static_cast<char*>(ptr), realSize);
    }
    
    return realSize;
}

bool IOThttp::httpsReq(const std::string &httpURL, const std::string &HttpBody, 
                       std::string &response, const std::string &extraHeaders, 
                       HttpMethod method) {
    std::lock_guard<std::mutex> lock(tokeMutex);
    
    CURL *curl = nullptr;
    CURLcode res;
    std::string responseData;
    bool success = false;
    
    // 初始化curl句柄
    curl = curl_easy_init();
    if (!curl) {
        VIOT_LOG_ERROR("curl_easy_init失败");
        return false;
    }
    
    // 设置URL
    std::string fullUrl = httpURL;
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    
    VIOT_LOG_INFO("HTTP请求: %s", fullUrl.c_str());
    
    // 设置HTTP方法
    switch (method) {
        case HttpMethod::GET:
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            VIOT_LOG_DEBUG("HTTP方法: GET");
            break;
            
        case HttpMethod::POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            if (!HttpBody.empty()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, HttpBody.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, HttpBody.length());
                VIOT_LOG_DEBUG("HTTP方法: POST, Body长度: %zu", HttpBody.length());
            }
            break;
            
        case HttpMethod::DELETE_:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            VIOT_LOG_DEBUG("HTTP方法: DELETE");
            break;
            
        default:
            VIOT_LOG_WARN("未知的HTTP方法");
            break;
    }
    
    // 设置HTTP头
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    if (!extraHeaders.empty()) {
        headers = curl_slist_append(headers, extraHeaders.c_str());
    }
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // 设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parseHttpsInfoCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // 设置超时时间
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    
    // 启用详细输出用于调试
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    // 允许重定向
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // 设置DNS解析
    curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 120L);
    
    // 设置SSL选项（如果是HTTPS）
    if (fullUrl.find("https://") == 0) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        
        // 如果有证书缓冲区，使用它
        if (cerBuff.buff && cerBuff.size > 0) {
            // 注意：这里假设cerBuff是PEM格式的证书
            // curl_easy_setopt(curl, CURLOPT_CAINFO, cerBuff.buff);
        }
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    
    // 执行HTTP请求
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        VIOT_LOG_ERROR("HTTP请求失败: %s (错误码: %d, URL: %s)", curl_easy_strerror(res), res, fullUrl.c_str());
        success = false;
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        VIOT_LOG_INFO("HTTP响应码: %ld, 响应长度: %zu", http_code, responseData.length());
        
        if (http_code >= 200 && http_code < 300) {
            VIOT_LOG_DEBUG("HTTP响应内容: %s", responseData.c_str());
            
            // 将响应数据传递给调用者
            response = responseData;
            
            // 将响应保存到cerBuff（如果需要的话）
            if (cerBuff.buff) {
                delete[] cerBuff.buff;
            }
            cerBuff.size = responseData.length();
            cerBuff.buff = new char[cerBuff.size + 1];
            std::memcpy(cerBuff.buff, responseData.c_str(), cerBuff.size);
            cerBuff.buff[cerBuff.size] = '\0';
            
            success = true;
        } else {
            VIOT_LOG_WARN("HTTP请求返回错误码: %ld", http_code);
            success = false;
        }
    }
    
    // 清理
    if (headers) {
        curl_slist_free_all(headers);
    }
    
    curl_easy_cleanup(curl);
    
    return success;
}

} // namespace http
} // namespace viot
