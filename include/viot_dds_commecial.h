#pragma once
#include <string>
#include <thread>
#include <atomic>
#include "blockingconcurrentqueue.h"
namespace viot {
namespace dds {

enum DDSIotComInfoType {
    ICIT_NODE_MOWER_INFO = 0,
    ICIT_NODE_NTRIP = 14,
    ICIT_VIOT_APP_COMMAND = 20,// viot 发布的app控制信息
};
struct IotComLargeInfoDDS {
    DDSIotComInfoType type;
    unsigned long len;
    char buf[8*1024]; // 假设最大数据长度为8*1024字节
};

class ViotDDSCommecial {
public:
    ViotDDSCommecial() = default;
    ~ViotDDSCommecial();
    void startProducerThread();
    void startGGAConsumerThread();
    bool waitDequeueGGA(std::string& out);
    bool waitDequeueDDS(IotComLargeInfoDDS& out);

    void push_data_queue(char* src,uint32_t len, DDSIotComInfoType type);
    void viot_dds_publish(char* src, uint32_t len, DDSIotComInfoType type);

private:
    moodycamel::BlockingConcurrentQueue<IotComLargeInfoDDS> dds_info_queue;
    moodycamel::BlockingConcurrentQueue<std::string> iot_dds_info_queue;
    std::thread producer_thread;
    std::thread consumer_thread;
    std::thread gga_consumer_thread;
    std::atomic<bool> running{false};
};

} // namespace dds
} // namespace viot