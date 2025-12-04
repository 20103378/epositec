#include "viot_ntrip_commercial.h"
#include "viot_http_commercial.h"
#include "viot_shared_commercial.h"
#include "viot_db_commercial.h"
#include "viot_log.h"
#include "viot_data_center.h"
#include <third_party/nlohmann/json.hpp>

namespace viot
{
    namespace ntrip
    {
        void viot_get_ntrip_info()
        {
            VIOT_LOG_INFO("自动切换到当前最优的供应商NTRIP账号");
            std::string rspbuff;
            double latitude, longitude;
            viot::utils::ShareData::GetInstance().getMowerPose(latitude, longitude);
            VIOT_PrintSaveLOG("gas_data json %.8f, %.8f", latitude, longitude);
            std::string rtk_url = NRTK_INFO_REQ;
            std::string g_rtk_info = "/thing/rtk/v1/allocate";
            std::string gps_data = "?latitude=";
            gps_data += std::to_string(latitude);
            gps_data += "&longitude=";
            gps_data += std::to_string(longitude);
            g_rtk_info += gps_data;

            viot::http::IOThttp v_http;
            if (v_http.httpsReq(rtk_url + gps_data, "", rspbuff, g_rtk_info, viot::http::IOThttp::HttpMethod::GET))
            {
                try
                {
                    nlohmann::json ntrip_info, rsp_json, rsp_data;
                    VIOT_PrintSaveLOG("HTTP响应: %s", rspbuff.c_str());
                    rsp_json = nlohmann::json::parse(rspbuff);
                    rsp_data = rsp_json["data"];
                    std::string expiresAt = std::to_string(rsp_data["expiresAt"].get<long>());
                    viot::db::viotDbCommercial::setNtripExpiresAt(expiresAt);
                    viot::utils::ShareData::GetInstance().setNtripExpiresTime(rsp_data["expiresAt"].get<long>());

                    ntrip_info["url"] = rsp_data["endpoint"].get<std::string>();
                    ntrip_info["user"] = rsp_data["userName"].get<std::string>();
                    ntrip_info["password"] = rsp_data["password"].get<std::string>();
                    ntrip_info["serviceCode"] = rsp_data["serviceCode"].get<int>();
                    viot::utils::ShareData::GetInstance().setNtripInfo(ntrip_info.dump());
                    viot::db::viotDbCommercial::setNtripInfo(ntrip_info.dump());
                    robot::utils::DataCenter::GetInstance().set<std::string>("viot/pub/ntrip_info", ntrip_info.dump());
                }
                catch (const nlohmann::json::exception &e)
                {
                    VIOT_PrintSaveLOG("https NRTK_INFO_RES parse error: %s", e.what());
                }
            }
        }
        bool release_ntrip_account()
        {
            VIOT_LOG_INFO("释放NTRIP账号");

            std::string rspbuff;
            viot::http::IOThttp v_http;
            if (v_http.httpsReq(NRTK_RELEASE_URL, "", rspbuff, "", viot::http::IOThttp::HttpMethod::GET))
            {
                viot::utils::ShareData::GetInstance().setGgaQuality(0);
                VIOT_LOG_INFO("NTRIP账号释放请求已发送");
                return true;
            }
            else
            {
                VIOT_LOG_WARN("NTRIP账号释放请求失败");
                return false;
            }
        }

        bool change_ntrip_account(int serviceCode)
        {
            VIOT_PrintSaveLOG("需要切换服务商账号:%d", serviceCode);
            std::string rspbuff;
            double latitude, longitude;
            viot::utils::ShareData::GetInstance().getMowerPose(latitude, longitude);
            VIOT_PrintSaveLOG("gas_data json %.8f, %.8f", latitude, longitude);
            std::string rtk_url = NRTK_INFO_REQ;
            std::string g_rtk_info = "/thing/rtk/v1/allocate";
            std::string gps_data = "?latitude=";
            gps_data += std::to_string(latitude);
            gps_data += "&longitude=";
            gps_data += std::to_string(longitude);
            gps_data += "&serviceCode=";
            gps_data += std::to_string(serviceCode);
            g_rtk_info += gps_data;

            viot::http::IOThttp v_http;
            if (v_http.httpsReq(rtk_url + gps_data, "", rspbuff, g_rtk_info, viot::http::IOThttp::HttpMethod::GET))
            {
                if (rspbuff.empty())
                {
                    VIOT_LOG_ERROR("服务器返回空响应，服务商代码 %d 可能不存在", serviceCode);
                    return false;
                }

                try
                {
                    nlohmann::json parse_root = nlohmann::json::parse(rspbuff);
                    VIOT_LOG_DEBUG("HTTP响应: %s", rspbuff.c_str());

                    if (parse_root.contains("data") && parse_root["data"].is_object())
                    {
                        VIOT_LOG_DEBUG("响应包含data字段");
                        nlohmann::json rsp_data = parse_root["data"];

                        long expiresAtLong = rsp_data["expiresAt"].get<long>();
                        std::string expiresAt = std::to_string(expiresAtLong);
                        viot::db::viotDbCommercial::setNtripExpiresAt(expiresAt);
                        viot::utils::ShareData::GetInstance().setNtripExpiresTime(expiresAtLong);

                        nlohmann::json ntrip_info;
                        ntrip_info["url"] = rsp_data["endpoint"].get<std::string>();
                        ntrip_info["user"] = rsp_data["userName"].get<std::string>();
                        ntrip_info["password"] = rsp_data["password"].get<std::string>();
                        ntrip_info["serviceCode"] = rsp_data["serviceCode"].get<int>();
                        viot::utils::ShareData::GetInstance().setNtripInfo(ntrip_info.dump());
                        viot::db::viotDbCommercial::setNtripInfo(ntrip_info.dump());
                        robot::utils::DataCenter::GetInstance().set<std::string>("viot/pub/ntrip_info", ntrip_info.dump());

                        VIOT_LOG_INFO("NTRIP信息切换成功: %s", ntrip_info.dump().c_str());
                        return true;
                    }
                    else
                    {
                        VIOT_LOG_ERROR("没有服务商代码 %d 的NTRIP信息", serviceCode);
                        return false;
                    }
                }
                catch (const std::exception &e)
                {
                    VIOT_LOG_ERROR("解析NTRIP信息失败: %s, 原始响应: '%s'", e.what(), rspbuff.c_str());
                    return false;
                }
            }
            else
            {
                VIOT_LOG_ERROR("HTTP请求失败，无法获取NTRIP信息");
                return false;
            }
        }

    } // namespace ntrip
} // namespace viot
