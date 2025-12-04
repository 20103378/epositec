#include "viot_dds_commecial.h"
#include "viot_shared_commercial.h"
#include "port_iot_diff_cutgo.h"
#include "viot_db_commercial.h"
#include "viot_log.h"
#include "viot_event_commercial.h"
#include "viot_ntrip_commercial.h"

#include <chrono>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <third_party/nlohmann/json.hpp>
#include <cstring>
#include <deque>
#include <thread>
#include <atomic>

namespace viot
{
    namespace dds
    {

        viotDDSCommecial::~viotDDSCommecial()
        {
            running = false;
            if (producer_thread.joinable())
                producer_thread.join();
            if (consumer_thread.joinable())
                consumer_thread.join();
            if (gga_consumer_thread.joinable())
                gga_consumer_thread.join();
        }

        static std::string make_gga(int count)
        {
            // 生成当前UTC时间
            std::time_t t = std::time(nullptr);
            std::tm *utc = std::gmtime(&t);
            std::ostringstream oss;
            oss << "$GPGGA,";
            oss << std::setfill('0') << std::setw(2) << utc->tm_hour
                << std::setw(2) << utc->tm_min
                << std::setw(2) << utc->tm_sec << ",";
            // 模拟经纬度
            oss << "3107.123,N,12128.456,E,";
            oss << (rand() % 10) << ","; // fix: 随机生产 0-5
            oss << "08,";                // numsats
            oss << "0.9,";               // hdop
            oss << "12.3,M,0.0,M,,*47";
            // 可加计数或其它字段
            oss << " // seq=" << count;
            return oss.str();
        }

        void viotDDSCommecial::startProducerThread()
        {
            running = true;
            producer_thread = std::thread([this]
                                          {

    nlohmann::json ntrip_info;
    ntrip_info["serviceCode"] = 0; // 示例服务码
    viot::utils::ShareData::GetInstance().setNtripInfo(ntrip_info.dump());
    viot::db::viotDbCommercial::setNtripInfo(ntrip_info.dump());
    std::cout << "初始化NTRIP信息: " << viot::utils::ShareData::GetInstance().getNtripInfo() << std::endl;
    std::string ntrip_info_db;
    viot::db::viotDbCommercial::getNtripInfo(ntrip_info_db);
    std::cout << "从数据库读取NTRIP信息: " << ntrip_info_db << std::endl;
    viot_event_start(); // 启动事件处理线程


        int count = 0;
        bool mower_sent = false;
        while (running) {
            // 生产GGA字符串
            std::string gga = make_gga(count);
            iot_dds_info_queue.enqueue(gga);
            // 生产IotComLargeInfoDDS结构体
            IotComLargeInfoDDS data;
            if (!mower_sent) {
                mower_sent = true;
                // ICIT_NODE_MOWER_INFO，结构体转JSON
                data.type = ICIT_NODE_MOWER_INFO;
                struct MowerDataCutGo {
                    int id;
                    std::string status;
                } mower;
                mower.id = 100 + count;
                mower.status = "working";
                nlohmann::json j;
                j["id"] = mower.id;
                j["status"] = mower.status;
                std::string json_str = j.dump();
                data.len = std::min((unsigned long)json_str.size(), sizeof(data.data));
                std::memcpy(data.data, json_str.data(), data.len);
            } else {
                // ICIT_GGA_INFO，原样填充
                data.type = ICIT_GGA_INFO;
                int content_len = std::snprintf(data.data, sizeof(data.data), "Sample data %d", count);
                data.len = content_len > 0 ? content_len : 0;
            }
            dds_info_queue.enqueue(data);
            ++count;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } });
        }

        bool viotDDSCommecial::waitDequeueGGA(std::string &out)
        {
            while (running)
            {
                if (iot_dds_info_queue.try_dequeue(out))
                    return true;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            return false;
        }
        bool app_map_start_info(const nlohmann::json &data, mower::port::iot::MowerDataCutGo &dataToMower, int &need_switch_account, int &serviceCode)
        { // 模拟解析逻辑
            if (data.contains("isNew") && data["isNew"].is_boolean())
            {
                if (data["isNew"].get<bool>())
                {
                    need_switch_account = false;
                    std::cout << "不需要切换 = " << need_switch_account << std::endl;
                }
                else
                {
                    if (data.contains("nrtk") && data["nrtk"].is_number())
                    {
                        int nrtk_value = data["nrtk"].get<int>();
                        if (nrtk_value != serviceCode)
                        {
                            need_switch_account = true;
                            serviceCode = nrtk_value;
                            VIOT_PrintSaveLOG("需要切换 :%d", need_switch_account);
                        }
                        else
                        {
                            need_switch_account = false;
                            VIOT_PrintSaveLOG("不需要切换 :%d", need_switch_account);
                        }
                    }
                }
                return true;
            }
            return false;
        }

        void viotDDSCommecial::startConsumerThread()
        {
            consumer_thread = std::thread([this]
                                          {
        while (running) {
            IotComLargeInfoDDS data;
            if (dds_info_queue.try_dequeue(data)) {
                switch (data.type) {
                        case ICIT_GGA_INFO:{
                            // std::cout << "[CONSUMER] GGA数据, len=" << data.len << ", data=" << std::string(data.data, data.len) << std::endl;
                        break;
                        }
                    case ICIT_NODE_MOWER_INFO:{
                         int need_switch_account = false;
                         int serviceCode = viot::utils::ShareData::GetInstance().getNtripServiceCode();
                         VIOT_PrintSaveLOG("当前NTRIP服务码=%d", serviceCode);

                            nlohmann::json parse_root;
                            nlohmann::json temp;
                            temp["isNew"] = false; // 示例服务码
                            temp["nrtk"] =  1; // 示例服务码
                            parse_root["data"] = temp;




                         mower::port::iot::MowerDataCutGo dataToMower;
                         memset(&dataToMower, 0, sizeof(dataToMower));
                         if(app_map_start_info(parse_root["data"],dataToMower,need_switch_account,serviceCode)){
                            if(need_switch_account){
                                 bool isReleased = viot::ntrip::release_ntrip_account();
                                 if(!isReleased){
                                     VIOT_PrintSaveLOG("释放服务商账号失败:%d", viot::utils::ShareData::GetInstance().getNtripServiceCode());
                                     VIOT_PrintSaveLOG("TODO: 放弃建图:%d", serviceCode);
                                     continue;
                                 }
                                bool ischange = viot::ntrip::change_ntrip_account(serviceCode);
                                if(!ischange){
                                    VIOT_PrintSaveLOG("切换服务商账号失败:%d", serviceCode);
                                    VIOT_PrintSaveLOG("TODO: 放弃建图:%d", serviceCode);
                                    viot::utils::ShareData::GetInstance().setNtripInfo("{}");
                                    viot::db::viotDbCommercial::setNtripInfo("{}");
                                    viot::utils::ShareData::GetInstance().setNtripExpiresTime(0);
                                    viot::db::viotDbCommercial::setNtripExpiresAt("0");
                                    viot_event_set_status(VIOT_EVENT_NEED_NTRIP_INFO); // 触发获取NTRIP信息事件
                                    continue;
                                }
                                std::thread([data, serviceCode]{
                                    VIOT_PrintSaveLOG("等待高质量定位数据...");
                                    viot::utils::ShareData::GetInstance().waitForGgaQuality();
                                    VIOT_PrintSaveLOG("获得高质量定位数据,TODO: 继续建图:%d", serviceCode);
                                }).detach();
                            }else {
                                VIOT_PrintSaveLOG("不需要切换服务商账号,TODO: 继续建图::%d", serviceCode);
                            }
    
                         }else{
                             VIOT_PrintSaveLOG("解析MOWER数据失败");
                         }
                        break;
                    }
                    default:
                    {
                        VIOT_PrintSaveLOG("未知DDS数据类型: %d", data.type);
                        break;
                    }
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } });
        }

        void viotDDSCommecial::startGGAConsumerThread()
        {
            gga_consumer_thread = std::thread([this]
                                              {
        while (running) {
            std::string gga;
            if (iot_dds_info_queue.try_dequeue(gga)) {
                // 解析fix字段
                int fix = -1;
                int comma_count = 0;
                for (size_t i = 0; i < gga.size(); ++i) {
                    if (gga[i] == ',') {
                        ++comma_count;
                        if (comma_count == 6) {
                            size_t start = i + 1;
                            size_t end = gga.find(',', start);
                            if (end != std::string::npos) {
                                std::string fix_str = gga.substr(start, end - start);
                                try {
                                    fix = std::stoi(fix_str);
                                } catch (...) {
                                    fix = -1;
                                }
                            }
                            break;
                        }
                    }
                }
                viot::utils::ShareData::GetInstance().setGgaQuality(fix);
                    std::cout << "[GGA CONSUMER] GGA数据: " << gga << std::endl;
                if (fix == 4 || fix == 5) {
                        std::cout << "[GGA CONSUMER] fix字段为 " << fix << "，为高质量定位！" << std::endl;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } });
        }

    } // namespace dds
} // namespace viot
