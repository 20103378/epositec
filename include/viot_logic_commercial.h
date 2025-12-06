#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include "blockingconcurrentqueue.h"
#include "port_iot_diff_cutgo.h"
#include "viot_dds_commecial.h"
#include "interface/viot_logic_interface.h"
#include "viot_app_inout_commercial.h"
namespace viot {
namespace logic {


class ViotLogicCommercial: public ViotLogicInterface {
public:
    ViotLogicCommercial();
   ~ViotLogicCommercial();
private:
std::shared_ptr<viot::plugs::ViotAppInOutInterface<mower::port::iot::MowerDataCutGo>> _app_inout_ptr;
void dds_info_handler(const viot::dds::IotComLargeInfoDDS &temp_data);
void thread_handle_aiot_dds(void);


protected:
   virtual void printVersion(void);
public:
virtual int init(void);

};

} // namespace logic
} // namespace viot