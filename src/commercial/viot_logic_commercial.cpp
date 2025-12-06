#include "viot_logic_commercial.h"
#include "viot_log.h"
#include "viot_mqtt_core_commercial.h"
#include "viot_shared_commercial.h"

extern viot::dds::ViotDDSCommecial viot_dds_shared;

namespace viot
{
    namespace logic
    {

        ViotLogicCommercial::ViotLogicCommercial()
        {
            VIOT_LOG_DEBUG("ViotLogicCommercial构造函数");
            _app_inout_ptr = std::make_shared<viot::plugs::ViotAppInOutCommercial>();
        }

        ViotLogicCommercial::~ViotLogicCommercial()
        {
            VIOT_LOG_DEBUG("ViotLogicCommercial析构函数");
        }

        void ViotLogicCommercial::printVersion(void)
        {
            VIOT_LOG_INFO("ViotLogicCommercial版本: 1.0.0");
            // TODO: 打印更详细的版本信息
        }

        void ViotLogicCommercial::dds_info_handler(const viot::dds::IotComLargeInfoDDS &temp_data)
        {
            VIOT_LOG_DEBUG("处理DDS信息, type=%d, len=%u", temp_data.type, temp_data.len);

            // TODO: 根据不同的数据类型处理DDS信息
            switch (temp_data.type)
            {
            case viot::dds::ICIT_NODE_NTRIP:
                VIOT_LOG_DEBUG("处理NTRIP信息");
                // TODO: 处理NTRIP数据
                break;
            case viot::dds::ICIT_NODE_MOWER_INFO:
            {
                VIOT_LOG_DEBUG("处理MOWER信息");
                uint32_t struct_len = sizeof(mower::port::iot::MowerDataCutGo);
                PUBMSGTYPE msgType = PUBMSGTYPE::MSGTYPE_PUB;
                int ret = -1;
                if (struct_len == temp_data.len)
                {
                    VIOT_LOG_DEBUG("MOWER信息长度匹配,%s",temp_data.buf);

                    mower::port::iot::MowerDataCutGo mower_to_app_info;
                    memset(&mower_to_app_info, 0, sizeof(mower::port::iot::MowerDataCutGo));
                    memcpy(&mower_to_app_info, temp_data.buf, struct_len);
                    viot::utils::SharedData::GetInstance()->setMowerDataCutGoInfo(&mower_to_app_info);
                    std::string send_app_str  = _app_inout_ptr->info_commandout_package(mower_to_app_info);
                    VIOT_LOG_DEBUG("发送到App的数据: %s", send_app_str.c_str());
                }

                break;
            }

            default:
                VIOT_LOG_WARN("未知的DDS信息类型: %d", temp_data.type);
                break;
            }
        }
        void ViotLogicCommercial::thread_handle_aiot_dds(void)
        {
            VIOT_LOG_DEBUG("启动AIOT DDS处理线程");
            while (1)
            {
                bool result = false;
                viot::dds::IotComLargeInfoDDS temp_data;
                result = viot_dds_shared.waitDequeueDDS(temp_data);
                if (result)
                {
                    dds_info_handler(temp_data);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        int ViotLogicCommercial::init(void)
        {
            VIOT_LOG_INFO("ViotLogicCommercial初始化");
            std::thread aiot_dds_handle = std::thread(&ViotLogicCommercial::thread_handle_aiot_dds, this);
            aiot_dds_handle.detach();

            // TODO: 实现初始化逻辑
            // 1. 初始化DDS通信
            // 2. 启动数据处理线程
            // 3. 注册回调函数

            return 0;
        }

    } // namespace logic
} // namespace viot
