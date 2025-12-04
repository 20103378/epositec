#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 事件类型枚举
typedef enum {
    VIOT_EVENT_NONE = 0,              // 无事件
    VIOT_EVENT_NEED_NTRIP_INFO,        // 需要NTRIP信息事件
    VIOT_EVENT_FINISH,              // 事件完成
    VIOT_EVENT_NTRIP_CONNECT,      // NTRIP连接事件
    VIOT_EVENT_NTRIP_DISCONNECT,       // NTRIP断开事件
    VIOT_EVENT_NTRIP_DATA_RECEIVED,    // NTRIP数据接收事件
    VIOT_EVENT_GGA_QUALITY_CHANGED,    // GGA质量变化事件
    VIOT_EVENT_SERVICE_SWITCH,         // 服务商切换事件
    VIOT_EVENT_MOWER_STATUS_CHANGED,   // 割草机状态变化事件
    VIOT_EVENT_CUSTOM                  // 自定义事件
    
} __attribute__((packed)) viot_event_status_t;


// 初始化事件系统
int viot_event_start();
void viot_event_set_status(viot_event_status_t status);

#ifdef __cplusplus
}
#endif
