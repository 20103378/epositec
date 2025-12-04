#pragma once
#include <string>
#include <thread>
#include <atomic>
#include "blockingconcurrentqueue.h"
namespace viot {
namespace dds {

enum DDSIotComInfoType {
    ICIT_NODE_MOWER_INFO = 0,
    ICIT_GGA_INFO = 1,
};
struct IotComLargeInfoDDS {
    DDSIotComInfoType type;
    unsigned long len;
    char data[8*1024]; // 假设最大数据长度为8*1024字节
};

class viotDDSCommecial {
public:
    viotDDSCommecial() = default;
    ~viotDDSCommecial();
    void startProducerThread();
    void startConsumerThread();
    void startGGAConsumerThread();
    bool waitDequeueGGA(std::string& out);
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