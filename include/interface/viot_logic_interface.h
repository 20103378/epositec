#pragma once
#include <string>
#include <thread>
#include <atomic>
namespace viot {
namespace logic {

class ViotLogicInterface {
public:
    ViotLogicInterface() = default;
    virtual ~ViotLogicInterface() = default;
protected:
    virtual void printVersion(void)=0;
private:
virtual int init(void)=0;

};

} // namespace logic
} // namespace viot