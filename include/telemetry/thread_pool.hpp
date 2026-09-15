#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

#include "telemetry/blocking_queue.hpp"

namespace telemetry {

class ThreadPool {
   public:
    explicit ThreadPool(std::size_t thread_count);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    bool submit(std::function<void()> task);
    void shutdown();

    [[nodiscard]] std::size_t thread_count() const noexcept { return workers_.size(); }

   private:
    void worker_loop();

    BlockingQueue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stopped_{false};
};

}  // namespace telemetry
