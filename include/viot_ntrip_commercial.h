#pragma once
#include <string>

namespace viot
{
    namespace ntrip
    {
        // 获取NTRIP账号信息并存储
        void viot_get_ntrip_info();

        // 释放当前NTRIP账号
        bool release_ntrip_account();

        // 切换到指定服务商的NTRIP账号
        // 参数: serviceCode - 服务商代码
        // 返回: true=成功, false=失败
        bool change_ntrip_account(int serviceCode);

    } // namespace ntrip
} // namespace viot
