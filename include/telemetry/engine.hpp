#pragma once
#include <boost/asio.hpp>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include "telemetry/alert_sink.hpp"
#include "telemetry/event_processor.hpp"
#include "telemetry/network/tcp_server.hpp"
#include "telemetry/network/udp_server.hpp"
#include "telemetry/parser.hpp"
#include "telemetry/rule.hpp"
#include "telemetry/thread_pool.hpp"
namespace telemetry {
struct TelemetryEngineConfig{std::uint16_t tcp_port{9000};std::uint16_t udp_port{9001};std::size_t worker_threads{4};std::string alerts_file;};
struct EngineStats{std::uint64_t received{0},parsed{0},parse_errors{0},alerts{0};};
class TelemetryEngine {
 public:
  explicit TelemetryEngine(TelemetryEngineConfig config); ~TelemetryEngine();
  TelemetryEngine(const TelemetryEngine&)=delete; TelemetryEngine& operator=(const TelemetryEngine&)=delete;
  void add_rule(std::unique_ptr<IRule> rule); void run(); void stop(); [[nodiscard]] EngineStats stats() const noexcept;
 private:
  void handle_message(std::string payload,Transport transport); std::unique_ptr<IAlertSink> create_sink(const std::string& alerts_file);
  TelemetryEngineConfig config_; boost::asio::io_context io_context_; boost::asio::signal_set signals_; ThreadPool workers_; TelemetryParser parser_; RuleEngine rules_; std::unique_ptr<AsyncAlertSink> sink_; std::unique_ptr<EventProcessor> processor_; std::unique_ptr<network::TcpServer> tcp_server_; std::unique_ptr<network::UdpServer> udp_server_; std::atomic<std::uint64_t> received_{0},parsed_{0},parse_errors_{0},alerts_{0}; std::atomic<bool> stopped_{false}; std::mutex log_mutex_;
};
}  // namespace telemetry
