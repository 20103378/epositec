#include "viot_dds_commecial.h"
#include "viot_logic_commercial.h"
#include <thread>
#include <chrono>
#include <iostream>
#include "viot_shared_commercial.h"

std::shared_ptr<viot::logic::ViotLogicCommercial> _logic_interface_ptr = nullptr;



int main() {
    viot_dds_shared.startProducerThread();
    viot_dds_shared.startGGAConsumerThread();

    // 初始化ViotLogicCommercial
    _logic_interface_ptr = std::make_shared<viot::logic::ViotLogicCommercial>();
    _logic_interface_ptr->init();

    // 主线程等待一段时间，演示生产和消费
    std::this_thread::sleep_for(std::chrono::seconds(10000));
    std::cout << "Main thread exit." << std::endl;
    // 析构时自动关闭线程
    return 0;
}