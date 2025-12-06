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

        ViotDDSCommecial::~ViotDDSCommecial()
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

        void ViotDDSCommecial::startProducerThread()
        {
            running = true;
            producer_thread = std::thread([this]
                                          {

    nlohmann::json ntrip_info;
    ntrip_info["serviceCode"] = 0; // 示例服务码
    viot::utils::SharedData::GetInstance()->setNtripInfo(ntrip_info.dump());
    viot::db::viotDBCommercial::setNtripInfo(ntrip_info.dump());
    std::cout << "初始化NTRIP信息: " << viot::utils::SharedData::GetInstance()->getNtripInfo() << std::endl;
    std::string ntrip_info_db;
    viot::db::viotDBCommercial::getNtripInfo(ntrip_info_db);
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
                data.type = ICIT_NODE_MOWER_INFO;
                mower::port::iot::MowerDataCutGo mower_info_tmp;
                memset(&mower_info_tmp, 0, sizeof(mower::port::iot::MowerDataCutGo));
                mower_info_tmp.head.msgId = count;
                mower_info_tmp.head.infoId = count;
                mower_info_tmp.head.cmd = mower::port::iot::ComCmdTypeCutGO::CMD_TYPE_LCD_MAPID;
                mower_info_tmp.head.msgType = mower::port::iot::ComMessageCutGoType::CUTGO_DATA_TYPE_UI_ASYNC;
                mower_info_tmp.head.ackFlag = false;
                mower_info_tmp.head.msgSender = mower::port::iot::ComMsgfrom::MSG_FROM_HMI;
                mower_info_tmp.head.msgReceiver = 1;
                int content_len = sizeof(mower::port::iot::MowerDataCutGo);
                std::memcpy(data.buf, &mower_info_tmp, content_len);
                data.len = content_len;
            } else {
                data.type = ICIT_NODE_NTRIP;
                int content_len = std::snprintf(data.buf, sizeof(data.buf), "Sample data %d", count);
                data.len = content_len > 0 ? content_len : 0;
            }
            dds_info_queue.enqueue(data);
            ++count;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } });
        }
        void ViotDDSCommecial::viot_dds_publish(char *src, uint32_t len, DDSIotComInfoType type)
        {
            VIOT_LOG_DEBUG("viot_dds_publish, type=%d, len=%u", type, len);
            
            if (type == ICIT_VIOT_APP_COMMAND) {
                mower::port::iot::MowerDataCutGo* mower_data = (mower::port::iot::MowerDataCutGo*)src;
                nlohmann::json j;
                j["head"]["msgId"] = mower_data->head.msgId;
                j["head"]["infoId"] = mower_data->head.infoId;
                j["head"]["cmd"] = static_cast<int>(mower_data->head.cmd);
                j["head"]["msgType"] = static_cast<int>(mower_data->head.msgType);
                j["head"]["ackFlag"] = mower_data->head.ackFlag;
                j["head"]["msgSender"] = static_cast<int>(mower_data->head.msgSender);
                j["head"]["msgReceiver"] = mower_data->head.msgReceiver;
                VIOT_LOG_DEBUG("发布MOWER_INFO内容: %s", j.dump().c_str());
            } else {
                std::string content(src, std::min(len, (uint32_t)100)); // 限制打印长度
                VIOT_LOG_DEBUG("发布内容: %s", content.c_str());
            }
        }
        void ViotDDSCommecial::push_data_queue(char *src, uint32_t len, DDSIotComInfoType type)
        {
            std::array<char, 8 * 1024> buffer;
            if (len > buffer.size())
            {
                VIOT_LOG_ERROR("DDS数据过大,len=%u", len);
                return;
            }
            std::memcpy(buffer.data(), src, len);
            IotComLargeInfoDDS self_data;
            self_data.type = type;
            self_data.len = len;
            std::memcpy(self_data.buf, buffer.data(), len);
            dds_info_queue.enqueue(self_data);
        }
        bool ViotDDSCommecial::waitDequeueGGA(std::string &out)
        {
            while (running)
            {
                if (iot_dds_info_queue.try_dequeue(out))
                    return true;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            return false;
        }
        bool ViotDDSCommecial::waitDequeueDDS(IotComLargeInfoDDS &out)
        {
            while (running)
            {
                if (dds_info_queue.try_dequeue(out))
                    return true;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            return false;
        }

        void ViotDDSCommecial::startGGAConsumerThread()
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
                viot::utils::SharedData::GetInstance()->setGgaQuality(fix);
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
