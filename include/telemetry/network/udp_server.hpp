#pragma once
#include <boost/asio.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <string>
namespace telemetry::network {
class UdpServer {
 public:
  using MessageHandler=std::function<void(std::string)>;
  UdpServer(boost::asio::io_context& io_context,std::uint16_t port,MessageHandler handler);
  void start(); void stop();
 private:
  void receive_next();
  boost::asio::ip::udp::socket socket_; boost::asio::ip::udp::endpoint remote_endpoint_; std::array<char,4096> buffer_{}; MessageHandler handler_;
};
}  // namespace telemetry::network
