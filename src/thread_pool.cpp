#include "telemetry/thread_pool.hpp"

#include <algorithm>
#include <utility>

namespace telemetry {

ThreadPool::ThreadPool(std::size_t thread_count) {
    thread_count = std::max<std::size_t>(1, thread_count);
    workers_.reserve(thread_count);
    for (std::size_t index = 0; index < thread_count; ++index) {
        workers_.emplace_back([this] { worker_loop(); });
    }
}

ThreadPool::~ThreadPool() { shutdown(); }

bool ThreadPool::submit(std::function<void()> task) {
    if (!task || stopped_.load(std::memory_order_acquire)) {
        return false;
    }
    return tasks_.push(std::move(task));
}

void ThreadPool::shutdown() {
    bool expected = false;
    if (!stopped_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    tasks_.close();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::worker_loop() {
    while (auto task = tasks_.wait_pop()) {
        try {
            (*task)();
        } catch (...) {
            // A task must not terminate the worker thread. Callers own task-level error reporting.
        }
    }
}

}  // namespace telemetry
