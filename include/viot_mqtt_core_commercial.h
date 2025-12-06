#pragma once
#include <string>
#include <thread>
#include <atomic>
#include "blockingconcurrentqueue.h"
#include "port_iot_diff_cutgo.h"
#include "interface/viot_logic_interface.h"
enum PUBMSGTYPE
{
    MSGTYPE_PUB = 0,       // 其他主动发布消息
    MSGTYPE_SYNC = 1,      // 同步消息
    MSGTYPE_PUB_PATH = 2, // 路径类消息
};