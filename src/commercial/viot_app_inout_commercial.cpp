#include "viot_app_inout_commercial.h"
#include "viot_log.h"
#include "viot_shared_commercial.h"
#include "viot_ntrip_commercial.h"
#include "viot_db_commercial.h"
#include "viot_event_commercial.h"
#include "viot_dds_commecial.h"
#include <third_party/nlohmann/json.hpp>

namespace viot
{
    namespace plugs
    {

        ViotAppInOutCommercial::ViotAppInOutCommercial()
        {
            VIOT_LOG_DEBUG("viotAppInOutCommercial构造函数");
        }
        ViotAppInOutCommercial::~ViotAppInOutCommercial()
        {
            VIOT_LOG_DEBUG("viotAppInOutCommercial析构函数");
        }
        void ViotAppInOutCommercial::setMapEvent(mapEvent event)
        {
            this->event = event;
        }
        void ViotAppInOutCommercial::appToAppCmd(ComCmdTypeCutGO cmd, ComMessageCutGoType msgType,
                                                 uint32_t msgId, uint32_t infoid,
                                                 ComStateReqAckCutGo ackCode)
        {
            VIOT_LOG_INFO(" cmd=%d, type=%d, msgId=%u, infoid=%u, ackCode=%d",
                          cmd, msgType, msgId, infoid, ackCode);

            mower::port::iot::MowerDataCutGo mower_info_tmp;
            mower_info_tmp.head.cmd = cmd;
            mower_info_tmp.head.msgSender = ComMsgfrom::MSG_FROM_HMI;
            mower_info_tmp.head.ackCode = ackCode;
            mower_info_tmp.head.msgType = msgType;
            mower_info_tmp.head.msgId = msgId;
            mower_info_tmp.head.infoId = infoid;
            viot_dds_shared.push_data_queue((char *)&mower_info_tmp, sizeof(mower_info_tmp), viot::dds::ICIT_NODE_MOWER_INFO);
        }
        void ViotAppInOutCommercial::lcdMapMatch(mower::port::iot::MowerDataCutGo *src, nlohmann::json &data)
        {
        }
    void ViotAppInOutCommercial::NoSendDataToApp(mower::port::iot::MowerDataCutGo* src,nlohmann::json &data){
        if(NULL == data){
            return;
        }
        data["sendmagic"] = 0x0a0b010c;
    }

        bool ViotAppInOutCommercial::app_map_start_info(const nlohmann::json &data, mower::port::iot::MowerDataCutGo &appdatatocg, int &need_switch_account, int &serviceCode)
        { // 模拟解析逻辑
            if (data.contains("isNew") && data["isNew"].is_boolean())
            {
                if (data["isNew"].get<bool>())
                {
                    need_switch_account = false;
                    std::cout << "不需要切换 = " << need_switch_account << std::endl;
                    appdatatocg.head.cmd = ComCmdTypeCutGO::CMD_TYPE_DEV_MAP_START_REQ;
                    appdatatocg.load.hmi.data.mapOpt.mapId = 0;
                    appdatatocg.load.hmi.data.mapOpt.isNew = true;
                    appdatatocg.load.hmi.data.mapOpt.serviceCode = serviceCode; // TODO添加服务码
                }
                else
                {
                    appdatatocg.load.hmi.data.mapOpt.isNew = false;
                    if (data.contains("nrtk") && data["nrtk"].is_number())
                    {
                        int nrtk_value = data["nrtk"].get<int>();
                        appdatatocg.load.hmi.data.mapOpt.serviceCode = nrtk_value; // TODO添加服务码
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
        nlohmann::json ViotAppInOutCommercial::infotypePraseMowerData(mower::port::iot::MowerDataCutGo *src, app_json_info_type &infoType, nlohmann::json &iotmsg)
        {
            std::string keystr;
            std::string valuestr;
            std::string pssstr;
            mapgps_point_t mapgps;
            nlohmann::json data;
            switch (src->head.cmd)
            {
            case ComCmdTypeCutGO::CMD_TYPE_LCD_MAPID: // 按键建图
            {
                int need_switch_account = false;
                int serviceCode = viot::utils::SharedData::GetInstance()->getNtripServiceCode();
                VIOT_PrintSaveLOG("当前NTRIP服务码=%d", serviceCode);

                nlohmann::json parse_root;
                nlohmann::json temp;
                temp["isNew"] = false; // 示例服务码
                temp["nrtk"] = 1;      // 示例服务码
                parse_root["data"] = temp;

                mower::port::iot::MowerDataCutGo dataToMower;
                memset(&dataToMower, 0, sizeof(dataToMower));
                if (app_map_start_info(parse_root["data"], dataToMower, need_switch_account, serviceCode))
                {
                    if (!need_switch_account)
                    {
                        VIOT_PrintSaveLOG("不需要切换服务商账号,TODO: 继续建图::%d", serviceCode);
                    }
                    else
                    {
                        // TODO 通知app供应商切换中
                        setMapEvent(mapEvent::EVENT_MAP_NRTK_SWITCHING);
                        appToAppCmd(ComCmdTypeCutGO::CMD_TYPE_MAP_EVENT,ComMessageCutGoType::CUTGO_DATA_TYPE_UI_ASYNC,0,0,ComStateReqAckCutGo::IOT_ACK_CODE_SUCCESS);
                       
                       
                        bool isReleased = viot::ntrip::release_ntrip_account();
                        if (isReleased)
                        {
                            
                            bool ischange = viot::ntrip::change_ntrip_account(serviceCode);
                            if (ischange)
                            {
                                VIOT_PrintSaveLOG("切换服务商账号成功:%d", serviceCode);
                                dataToMower.head.cmd = ComCmdTypeCutGO::CMD_TYPE_LCD_MAPID;
                                dataToMower.head.ackFlag = false;
                                dataToMower.head.msgReceiver = 2;
                                std::thread([this, dataToMower, serviceCode]
                                            {
                                    VIOT_PrintSaveLOG("等待高质量定位数据...");
                                    viot::utils::SharedData::GetInstance()->waitForGgaQuality();
                                    setMapEvent(mapEvent::EVENT_MAP_MAP_START);
                                    viot_dds_shared.viot_dds_publish((char *)&dataToMower, sizeof(mower::port::iot::MowerDataCutGo), viot::dds::ICIT_VIOT_APP_COMMAND);
                                    appToAppCmd(ComCmdTypeCutGO::CMD_TYPE_MAP_EVENT,ComMessageCutGoType::CUTGO_DATA_TYPE_UI_ASYNC,0,0,ComStateReqAckCutGo::IOT_ACK_CODE_SUCCESS);
                                    VIOT_PrintSaveLOG("获得高质量定位数据,TODO: 继续建图:%d", serviceCode); })
                                    .detach();
                            }
                            else
                            {
                                VIOT_PrintSaveLOG("切换服务商账号失败:%d", serviceCode);
                                VIOT_PrintSaveLOG("TODO: 放弃建图:%d", serviceCode);
                                viot::utils::SharedData::GetInstance()->setNtripInfo("{}");
                                viot::db::viotDBCommercial::setNtripInfo("{}");
                                viot::utils::SharedData::GetInstance()->setNtripExpiresTime(0);
                                viot::db::viotDBCommercial::setNtripExpiresAt("0");
                                viot_event_set_status(VIOT_EVENT_NEED_NTRIP_INFO); // 触发获取NTRIP信息事件
                            }
                        }
                        else
                        {
                            VIOT_PrintSaveLOG("释放服务商账号失败:%d", viot::utils::SharedData::GetInstance()->getNtripServiceCode());
                            VIOT_PrintSaveLOG("TODO: 放弃建图:%d", serviceCode);
                        }
                    }
                }
                else
                {
                    VIOT_PrintSaveLOG("解析MOWER数据失败");
                }
                NoSendDataToApp(src,data);
                break;
            }
            case ComCmdTypeCutGO::CMD_TYPE_DEV_MAP_UPDATE_REQ: // lcd请求匹配地图数据
            {
                lcdMapMatch(src, data);
                break;
            }
            case ComCmdTypeCutGO::CMD_TYPE_MAP_EVENT: // 建图事件
            {
                VIOT_PrintSaveLOG("收到建图事件cmd: %d", static_cast<int>(src->head.cmd));
                infoType = APP_JSON_INFO_APP_MAP_T_EVENT;
                data["event"] = getMapEvent();
                break;
            }

            default:
                break;
            }
            return data;
        }

        void ViotAppInOutCommercial::info_response_parse(const char *src)
        {
        }
        std::string ViotAppInOutCommercial::info_commandin_parse(const char *src, ViotAppInfoType type)
        {
            std::string res = "";

            mower::port::iot::MowerDataCutGo app_data;
            memset(&app_data, 0, sizeof(mower::port::iot::MowerDataCutGo));
            char buffer[4 * 1024];
            std::map<std::string, std::string>::iterator it;
            uint32_t index = 0;
            numAttr = 0;
            bool parseRes = true;
            switch (type)
            {
            case ViotAppInfoType::CONFIG:
            {
                nlohmann::json parse_root = nlohmann::json::parse(src);
                // parseRes = parse_commandin(app_data, parse_root, type); // TODO: Implement parse_commandin
                if (!parseRes)
                {
                    app_data.head.cmd = ComCmdTypeCutGO::CMD_TYPE_IOT_PARSE_DATA_ERROR;
                    app_data.head.msgType = ComMessageCutGoType::CUTGO_DATA_TYPE_UI_SYNC;
                    app_data.head.ackFlag = true;
                    app_data.head.msgReceiver = static_cast<uint32_t>(ComMsgfrom::MSG_FROM_HMI);
                    app_data.head.msgSender = ComMsgfrom::MSG_FROM_IOT;
                    app_data.head.ackCode = ComStateReqAckCutGo::IOT_ACK_CODE_MQTT_PARAM_ERROR;
                    viot_dds_shared.push_data_queue((char *)&app_data, sizeof(app_data), viot::dds::ICIT_NODE_MOWER_INFO);
                }
                else
                {
                    VIOT_LOG_ERROR("解析CONFIG数据失败");
                }
                break;
            }
            case ViotAppInfoType::JOB:
            {
                break;
            }
            case ViotAppInfoType::TOKEN:
            {
                break;
            }
            default:
            {
                VIOT_LOG_WARN("未知App信息类型: %d", static_cast<int>(type));
            }
            }

            return res;
        }

        nlohmann::json ViotAppInOutCommercial::commandout_dat(const mower::port::iot::MowerDataCutGo *src)
        {

            app_json_info_type infoType = APP_JSON_INFO_IDLE;
            nlohmann::json toAppMsg;
            nlohmann::json iotMsg;
            mower::port::iot::MowerDataCutGo srcTmp;
            memcpy(&srcTmp, src, sizeof(mower::port::iot::MowerDataCutGo));

            nlohmann::json data = infotypePraseMowerData(&srcTmp, infoType, iotMsg);
            
            toAppMsg["data"] = data;

            toAppMsg["sn"] = "kr800"; // TODO: 替换为实际设备序列号
            toAppMsg["infoType"] = infoType;

            if(infoType == APP_JSON_INFO_NRTK_SWITCHING){
               VIOT_PrintSaveLOG("infoType 供应商切换=%d", infoType);
            }

            if (MSG_SERVICE_PERIODICITY == infoType)
            {
                toAppMsg["iot"] = iotMsg;
            }
            toAppMsg["message"] = std::string(srcTmp.head.descriprionInfo, strlen(srcTmp.head.descriprionInfo));
            if (srcTmp.head.msgType == ComMessageCutGoType::CUTGO_DATA_TYPE_UI_SYNC)
            {
                toAppMsg["msgId"] = srcTmp.head.msgId;
            }
            else
            {
                toAppMsg["msgId"] = gAppOutMessageIdIndex++;
            }
            toAppMsg["timestamp"] = std::time(nullptr);
            toAppMsg["code"] = srcTmp.head.ackCode > 0 ? static_cast<int>(srcTmp.head.ackCode) : ComStateReqAckCutGo::IOT_ACK_CODE_SUCCESS;
            nlohmann::json datajson = toAppMsg.at("data");
            if (datajson.contains("sendmagic"))
            {
                nlohmann::json nojson;
                toAppMsg = nojson;
                return toAppMsg;
            }
            return toAppMsg;
        }
        std::string ViotAppInOutCommercial::info_commandout_package(const mower::port::iot::MowerDataCutGo &src)
        {
            std::string res = "";
            _data = src;
            nlohmann::json send_root;
            nlohmann::json toAppMsg = commandout_dat(&src);
            send_root["sync"] = (_data.head.msgType == ComMessageCutGoType::CUTGO_DATA_TYPE_UI_SYNC) ? 1 : 0;
            send_root["dat"] = toAppMsg;
            return send_root.dump();
        }

    } // namespace plugs
} // namespace viot
