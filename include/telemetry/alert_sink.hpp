#pragma once

#include <atomic>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "telemetry/blocking_queue.hpp"
#include "telemetry/rule.hpp"

namespace telemetry {

class IAlertSink {
   public:
    virtual ~IAlertSink() = default;
    virtual void publish(const Alert& alert) = 0;
};

class ConsoleAlertSink final : public IAlertSink {
   public:
    void publish(const Alert& alert) override;
   private:
    std::mutex mutex_;
};

class JsonFileAlertSink final : public IAlertSink {
   public:
    explicit JsonFileAlertSink(const std::string& path);
    void publish(const Alert& alert) override;
   private:
    std::ofstream output_;
    std::mutex mutex_;
};

class AsyncAlertSink final : public IAlertSink {
   public:
    explicit AsyncAlertSink(std::unique_ptr<IAlertSink> downstream);
    ~AsyncAlertSink() override;
    AsyncAlertSink(const AsyncAlertSink&) = delete;
    AsyncAlertSink& operator=(const AsyncAlertSink&) = delete;
    void publish(const Alert& alert) override;
    void stop();
   private:
    void writer_loop();
    std::unique_ptr<IAlertSink> downstream_;
    BlockingQueue<Alert> queue_;
    std::thread writer_;
    std::atomic<bool> stopped_{false};
};

}  // namespace telemetry
