#pragma once
#include "blockingconcurrentqueue.h"
#include <string>
#include <thread>
#include <atomic>
#include <cstring>
namespace mower {
namespace port {
namespace iot {
enum ComStateReqAckCutGo : uint16_t {
    IOT_ACK_CODE_IDLE = 0,
    IOT_ACK_CODE_SUCCESS = 200,//所有模块成功的应答
    IOT_ACK_CODE_MQTT_PARAM_ERROR = 1003,//mqtt参数错误
    IOT_REQ_CODE_SUCCESS = 1,
};
//上报类型
enum ComMessageCutGoType {
    CUTGO_DATA_TYPE_IDLE = 0,
    CUTGO_DATA_TYPE_UI_SYNC, //同步消息
    CUTGO_DATA_TYPE_UI_ASYNC, //异步消息
};
enum class ComCmdTypeCutGO : uint16_t {
    CMD_TYPE_NONE = 0,  
    CMD_TYPE_IOT_PARSE_DATA_ERROR = 3, // 收到mqtt数据解析失败，不符合协议，发送给aiot-然后aiot转发到viot
    CMD_TYPE_DEV_MAP_UPDATE_REQ = 510, // lcd请求匹配地图数据
    CMD_TYPE_LCD_MAPID = 514,           // 按键请求建图
    CMD_TYPE_MAP_EVENT = 516, // 建图结束
    CMD_TYPE_DEV_MAP_START_REQ = 101, 
};
        
struct CutGoMapOperate{
    CutGoMapOperate(){}
    ~CutGoMapOperate(){}
    CutGoMapOperate& operator=(const CutGoMapOperate& temp_data){ 
        mapId = temp_data.mapId;
        isNew = temp_data.isNew;
        mode = temp_data.mode;
        return *this;
    }
    int mapId;
    bool isNew;
    int mode;
    int serviceCode;// ntrip服务商代码

};
struct IotBusCutGoUiInfo{
       IotBusCutGoUiInfo(){}
       ~IotBusCutGoUiInfo(){}
       struct CutGoMapOperate mapOpt;//建图操作
};
struct  IotBusCutGoHmi {
    IotBusCutGoHmi(){}
    ~IotBusCutGoHmi(){}
        IotBusCutGoHmi& operator=(const IotBusCutGoHmi& temp_data){ 
        data = temp_data.data;
        return *this;
    }
    struct IotBusCutGoUiInfo data; //ui信息
};
enum class ComMsgfrom : uint8_t {
    MSG_FROM_NONE = 0,
    MSG_FROM_IOT,
    MSG_FROM_HMI,
    MSG_FROM_MAP,
    MSG_FROM_LOG,
    MSG_FROM_FI,
    MSG_FROM_AUTOMOWER,
    MSG_FROM_WIFI,
    MSG_FROM_UCM
};
struct  IOTMsgHead
{
    IOTMsgHead(){
        msgId = 0;
        infoId = 0;
        msgType = ComMessageCutGoType::CUTGO_DATA_TYPE_IDLE;
        cmd = ComCmdTypeCutGO::CMD_TYPE_DEV_MAP_UPDATE_REQ;
        ackFlag = false;
        msgSender = ComMsgfrom::MSG_FROM_NONE;
        msgReceiver = 0;
        ackCode = ComStateReqAckCutGo::IOT_ACK_CODE_IDLE;
        memset(descriprionInfo,0,sizeof(descriprionInfo));
        exceptionCode = 0;
    }
    ~IOTMsgHead() = default;
    IOTMsgHead& operator=(const IOTMsgHead& temp_data){ 
        msgId = temp_data.msgId;
        infoId = temp_data.infoId;
        msgType = temp_data.msgType;
        cmd = temp_data.cmd;
        ackFlag = temp_data.ackFlag;
        msgSender = temp_data.msgSender;
        msgReceiver = temp_data.msgReceiver;
        ackCode = temp_data.ackCode;
        memcpy(descriprionInfo,temp_data.descriprionInfo,sizeof(descriprionInfo));
        exceptionCode = temp_data.exceptionCode;
        return *this;
    }
    uint32_t msgId;// 消息id
    uint32_t infoId;//InfoType值
    ComMessageCutGoType msgType;//数据类型，同步/异步
    ComCmdTypeCutGO cmd;//控制的命令类型
    bool ackFlag;//是否需要回复消息
    ComMsgfrom msgSender;//消息发送者
    uint32_t msgReceiver;//消息接收者
    ComStateReqAckCutGo ackCode;//响应码
    char descriprionInfo[20];
    uint32_t exceptionCode;
};
union msgPaylod
{
    msgPaylod(){}
    ~msgPaylod(){}

    IotBusCutGoHmi hmi;

};
struct MowerDataCutGo
{
    MowerDataCutGo(){
        memset(&load, 0, sizeof(load));
    }
    ~MowerDataCutGo() = default;
    MowerDataCutGo& operator=(const MowerDataCutGo& temp_data){ 
        head = temp_data.head;
        return *this;
    }
   IOTMsgHead head;
   msgPaylod load;
};

} 
}
} // namespace mower