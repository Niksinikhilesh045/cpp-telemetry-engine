#pragma once
#include <boost/asio.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
namespace telemetry::network {
using MessageHandler = std::function<void(std::string)>;
class TcpServer {
 public:
  TcpServer(boost::asio::io_context& io_context, std::uint16_t port, MessageHandler handler);
  void start(); void stop();
 private:
  void do_accept();
  boost::asio::ip::tcp::acceptor acceptor_;
  MessageHandler handler_;
};
}  // namespace telemetry::network
