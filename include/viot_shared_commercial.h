
#pragma once
#include "utils/singleton.h"
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <cstring>
#include "port_iot_diff_cutgo.h"
#include "viot_dds_commecial.h"
#include "viot_http_commercial.h"
extern viot::http::IOThttp v_http;
extern viot::dds::ViotDDSCommecial viot_dds_shared;
namespace viot
{
    namespace utils
    {

        class SharedData : public robot::utils::Singleton<SharedData>
        {
        public:
            SharedData() = default;
            ~SharedData() = default;

        protected:
            SharedData(const SharedData &) = delete;
            SharedData &operator=(const SharedData &) = delete;

        private:
         
            int gga_quality = 0; // 示例共享数据成员
            mutable std::mutex gga_quality_mtx;
            std::condition_variable gga_quality_cv;


            //
            std::string ntripInfo;
            long ntrip_expires_time = 0; 
            double latitude = 31.123456;
            double longitude = 121.654321;  // 模拟GPS坐标
            mower::port::iot::MowerDataCutGo mower_to_app_info;
            //

        public:
                int getGgaQuality() const;
                void setGgaQuality(int val);
                void waitForGgaQuality();
                int getNtripServiceCode() const;


                void setNtripExpiresTime(long expiresAt) {
                    this->ntrip_expires_time = expiresAt;
                }
                long getNtripExpiresTime() const {
                    return this->ntrip_expires_time;
                }

                void setNtripInfo(std::string ntrip) {
                    this->ntripInfo = ntrip;
                }
                std::string getNtripInfo() const {
                    return this->ntripInfo;
                }
                void setMowerPose(double lat, double lon) {
                    this->latitude = lat;
                    this->longitude = lon;
                }
                void getMowerPose(double &lat, double &lon) const {
                    lat = this->latitude;
                    lon = this->longitude;
                }
                void setMowerDataCutGoInfo( mower::port::iot::MowerDataCutGo* info) {
                    memcpy(&this->mower_to_app_info, info, sizeof(mower::port::iot::MowerDataCutGo));
                }
                void getMowerDataCutGoInfo(mower::port::iot::MowerDataCutGo* buf) const {
                   memcpy(buf, &this->mower_to_app_info, sizeof(mower::port::iot::MowerDataCutGo));
                }
        
        };

    } // namespace utils
} // namespace viot