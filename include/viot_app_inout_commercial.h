#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <cstring>
#include <ctime>
#include <map>
#include "blockingconcurrentqueue.h"
#include "port_iot_diff_cutgo.h"
#include "interface/viot_app_inout_interface.h"
#include <third_party/nlohmann/json.hpp>

// Using declarations for convenience
using mower::port::iot::ComCmdTypeCutGO;
using mower::port::iot::ComMessageCutGoType;
using mower::port::iot::ComStateReqAckCutGo;
using mower::port::iot::ComMsgfrom;

namespace viot {
namespace plugs {

static uint32_t gAppOutMessageIdIndex = 1;

typedef struct _mapgps_point_t {
    double lat;
    double lon;
} __attribute__((__packed__)) mapgps_point_t;


typedef enum _app_json_info_type {
      APP_JSON_INFO_IDLE = 0,
      MSG_SERVICE_PERIODICITY = 100,//周期性数据
      APP_JSON_INFO_APP_MAP_T_EVENT = 331, // 建图事件
      APP_JSON_INFO_NRTK_SWITCHING = 22561, // ntrip切换中
} __attribute__((packed)) app_json_info_type;

enum class mapEvent : uint8_t {
    EVENT_MAP_MAP_START = 10,//建图，大开始
    EVENT_MAP_MAP_FINISH = 9,//建图，大结束,
    EVENT_MAP_NRTK_SWITCHING = 25,//ntrip切换中
};
enum class mapEventReason : uint16_t {
    EVENT_REASON_NONE = 0,
    EVENT_REASON_NRTK_ACCOUNT_SWITCH = 1, // ntrip账号切换
};  

class ViotAppInOutCommercial: public ViotAppInOutInterface<mower::port::iot::MowerDataCutGo> {
public:
    ViotAppInOutCommercial();
    virtual ~ViotAppInOutCommercial();
private:
    mower::port::iot::MowerDataCutGo _data;
    mapEvent event;
    uint32_t numAttr = 0;
    void setMapEvent(mapEvent event);
    mapEvent getMapEvent() { return this->event; }   
    nlohmann::json infotypePraseMowerData(mower::port::iot::MowerDataCutGo* src ,app_json_info_type &infoType,nlohmann::json &iotmsg);
    void appToAppCmd(ComCmdTypeCutGO cmd,ComMessageCutGoType type,uint32_t msgId,uint32_t infoid,ComStateReqAckCutGo ackCode);
    nlohmann::json commandout_dat(const mower::port::iot::MowerDataCutGo *src);
    void lcdMapMatch(mower::port::iot::MowerDataCutGo* src,nlohmann::json &data);
    void NoSendDataToApp(mower::port::iot::MowerDataCutGo* src,nlohmann::json &data);
    bool app_map_start_info(const nlohmann::json &data, mower::port::iot::MowerDataCutGo &dataToMower, int &need_switch_account, int &serviceCode);
public:
   virtual void info_response_parse(const char* src);
   virtual std::string info_commandin_parse(const char* src,ViotAppInfoType type);//app->iot
   virtual std::string info_commandout_package(const mower::port::iot::MowerDataCutGo& src);//iot->app
};

} // namespace plugs
} // namespace viot