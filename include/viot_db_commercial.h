#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include "utils/singleton.h"

namespace viot {
    namespace db
    {
        class viotDBCommercial
        {
        public:
            viotDBCommercial();
            ~viotDBCommercial();
        public:
        static int setNtripExpiresAt(const std::string &NtripExpiresAt);
        static int getNtripExpiresAt(std::string &NtripExpiresAt);
        static int setNtripInfo(const std::string &NtripInfo);
        static int getNtripInfo(std::string &NtripInfo);


        private:
        };
    } // namespace db
} // namespace viot
