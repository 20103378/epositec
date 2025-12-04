#pragma once
#include "blockingconcurrentqueue.h"
#include <string>
#include <thread>
#include <atomic>
#include <cstring>
namespace mower {
namespace port {
namespace iot {

enum ComCmdTypeCutGO : uint16_t {
    CMD_TYPE_DEV_MAP_UPDATE_REQ = 510, // lcd请求匹配地图数据
    CMD_TYPE_LCD_MAPID = 514           // 按键请求建图
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

struct  IOTMsgHead
{
    IOTMsgHead(){

    }
    ~IOTMsgHead() = default;

    ComCmdTypeCutGO cmd;

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