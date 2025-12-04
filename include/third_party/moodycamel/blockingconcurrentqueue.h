// Placeholder for moodycamel BlockingConcurrentQueue
// Please download the official header from:
// https://github.com/cameron314/concurrentqueue/blob/master/blockingconcurrentqueue.h
// and replace this file.

#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace moodycamel {
// This is NOT a real implementation, just a placeholder for build/test.
template<typename T>
class BlockingConcurrentQueue {
public:
    void enqueue(const T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        q_.push(item);
        cv_.notify_one();
    }
    bool try_dequeue(T& out) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (q_.empty()) return false;
        out = q_.front();
        q_.pop();
        return true;
    }
    void wait_dequeue(T& out) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [&]{ return !q_.empty(); });
        out = q_.front();
        q_.pop();
    }
private:
    std::queue<T> q_;
    std::mutex mtx_;
    std::condition_variable cv_;
};
} // namespace moodycamel
