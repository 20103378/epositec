#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <cstdint>
#include "utils/singleton.h"

#define NRTK_INFO_REQ "http://172.19.16.1:8080/thing/rtk/v1/allocate"
#define NRTK_RELEASE_URL "http://172.19.16.1:8080/thing/rtk/v1/release"
namespace viot
{
    namespace http
    {
        struct httpsBuff
        {
            uint32_t size;
            char *buff;
            
            httpsBuff() : size(0), buff(nullptr) {}
            ~httpsBuff() {
                if (buff) {
                    delete[] buff;
                    buff = nullptr;
                }
            }
        };
        
        class IOThttp
        {
        public:
            enum class HttpMethod
            {
                GET,
                POST,
                DELETE_
            };
            
            IOThttp();
            ~IOThttp();
            
            bool httpsReq(const std::string &httpURL, const std::string &HttpBody, 
                         std::string &response, const std::string &extraHeaders = "", 
                         HttpMethod method = HttpMethod::POST);
                         
        private:
            httpsBuff cerBuff;
            static std::mutex tokeMutex;
            static size_t parseHttpsInfoCallback(void *ptr, size_t size, size_t nmemb, void *userdata);
        };

    } // namespace http
} // namespace viot
