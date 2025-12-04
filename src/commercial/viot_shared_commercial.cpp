#include "viot_shared_commercial.h"
#include <third_party/nlohmann/json.hpp>
#include "viot_http_commercial.h"


viot::http::IOThttp v_http;
namespace viot {
namespace utils {

int ShareData::getGgaQuality() const {
    std::lock_guard<std::mutex> lock(gga_quality_mtx);
    return gga_quality;
}
void ShareData::setGgaQuality(int val) {
    {
        std::lock_guard<std::mutex> lock(gga_quality_mtx);
        gga_quality = val;
    }
    if (val == 4 || val == 5) {
       gga_quality_cv.notify_all();
    }
}
void ShareData::waitForGgaQuality() {
    std::unique_lock<std::mutex> lock(gga_quality_mtx);
    gga_quality_cv.wait(lock, [this]{ return gga_quality == 4 || gga_quality == 5; });
}
 int ShareData::getNtripServiceCode() const {
    int service_code = -1;
    if (!ntripInfo.empty() && ntripInfo[0] == '{') {
        nlohmann::json parse_root = nlohmann::json::parse(this->ntripInfo);
        if(parse_root.contains("serviceCode")){
           service_code = parse_root["serviceCode"].get<int>();
        }
    } else {
        std::cout << "NtripInfo不是JSON格式，无法解析服务码" << std::endl;
    }
     return service_code;
 }

} // namespace utils
} // namespace viot
