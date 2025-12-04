
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


            std::string ntripInfo; 
            
            int gga_quality = 0; // 示例共享数据成员
            mutable std::mutex gga_quality_mtx;
            std::condition_variable gga_quality_cv;

        public:
                int getGgaQuality() const;
                void setGgaQuality(int val);
                void waitForFix45();

                void setNtripInfo(std::string ntrip) {
                    this->ntripInfo = ntrip;
                }
                std::string getNtripInfo() {
                    return this->ntripInfo;
                }
                int getNtripServiceCode() const;
        };

    } // namespace utils
} // namespace viot