#include "telemetry/alert_sink.hpp"

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace telemetry {
namespace {
std::string escape_json(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += ch;
        }
    }
    return escaped;
}
void write_json(std::ostream& output, const Alert& alert) {
    output << std::fixed << std::setprecision(2) << "{\"device\":\"" << escape_json(alert.device_id)
           << "\",\"metric\":\"" << escape_json(alert.metric) << "\",\"value\":" << alert.observed_value
           << ",\"condition\":\"" << to_string(alert.comparison) << "\",\"threshold\":" << alert.threshold
           << ",\"severity\":\"" << to_string(alert.severity) << "\",\"timestamp\":" << alert.timestamp_ms
           << ",\"transport\":\"" << to_string(alert.transport) << "\"}";
}
}  // namespace

void ConsoleAlertSink::publish(const Alert& alert) {
    std::lock_guard<std::mutex> lock(mutex_);
    write_json(std::cout, alert);
    std::cout << '\n';
}

JsonFileAlertSink::JsonFileAlertSink(const std::string& path) : output_(path, std::ios::out | std::ios::app) {
    if (!output_) throw std::runtime_error("unable to open alert output file: " + path);
}

void JsonFileAlertSink::publish(const Alert& alert) {
    std::lock_guard<std::mutex> lock(mutex_);
    write_json(output_, alert);
    output_ << '\n';
    output_.flush();
}

AsyncAlertSink::AsyncAlertSink(std::unique_ptr<IAlertSink> downstream)
    : downstream_(std::move(downstream)), writer_([this] { writer_loop(); }) {
    if (!downstream_) throw std::invalid_argument("downstream alert sink must not be null");
}
AsyncAlertSink::~AsyncAlertSink() { stop(); }
void AsyncAlertSink::publish(const Alert& alert) {
    if (!stopped_.load(std::memory_order_acquire)) queue_.push(alert);
}
void AsyncAlertSink::stop() {
    bool expected = false;
    if (!stopped_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) return;
    queue_.close();
    if (writer_.joinable()) writer_.join();
}
void AsyncAlertSink::writer_loop() {
    while (auto alert = queue_.wait_pop()) downstream_->publish(*alert);
}

}  // namespace telemetry
