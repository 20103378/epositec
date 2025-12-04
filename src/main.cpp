#include "viot_dds_commecial.h"
#include <thread>
#include <chrono>
#include <iostream>

int main() {
    viot::dds::viotDDSCommecial dds;
    dds.startProducerThread();
    dds.startConsumerThread();
    dds.startGGAConsumerThread();

    // 主线程等待一段时间，演示生产和消费
    std::this_thread::sleep_for(std::chrono::seconds(10000));
    std::cout << "Main thread exit." << std::endl;
    // 析构时自动关闭线程
    return 0;
}