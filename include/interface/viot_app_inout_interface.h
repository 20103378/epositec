#pragma once
#include <string>
#include <thread>
#include <atomic>
namespace viot {
namespace plugs {
enum class ViotAppInfoType: uint8_t {
    IDLE = 0,
    COMMAND,
    CONFIG,
    JOB,
    TOKEN
};
template<typename T>
class ViotAppInOutInterface {
public:
    ViotAppInOutInterface() = default;
    virtual ~ViotAppInOutInterface() = default;
public:
   virtual void info_response_parse(const char* src) = 0;
   virtual std::string info_commandin_parse(const char* src,ViotAppInfoType type) = 0;
   virtual std::string info_commandout_package(const T& src) = 0;

};

} // namespace plugs
} // namespace viot