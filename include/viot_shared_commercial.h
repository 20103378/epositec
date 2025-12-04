
#pragma once
#include "utils/singleton.h"
#include <mutex>
#include <condition_variable>
#include <iostream>
namespace viot
{
    namespace utils
    {

        class ShareData : public robot::utils::Singleton<ShareData>
        {
        public:
            ShareData() = default;
            ~ShareData() = default;

        protected:
            ShareData(const ShareData &) = delete;
            ShareData &operator=(const ShareData &) = delete;

        private:

            int gga_quality = 0; // 示例共享数据成员
            mutable std::mutex gga_quality_mtx;
            std::condition_variable gga_quality_cv;


            //
            std::string ntripInfo;
            long ntrip_expires_time = 0; 
            double latitude = 31.123456;
            double longitude = 121.654321;  // 模拟GPS坐标
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
        };

    } // namespace utils
} // namespace viot