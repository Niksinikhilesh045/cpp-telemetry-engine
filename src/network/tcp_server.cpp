#include <utility>
#include "telemetry/network/tcp_server.hpp"
#include <iostream>
namespace telemetry::network { namespace {
class TcpSession : public std::enable_shared_from_this<TcpSession> {
 public:
  TcpSession(boost::asio::ip::tcp::socket socket, MessageHandler handler): socket_(std::move(socket)), handler_(std::move(handler)) {}
  void start(){ read_line(); }
 private:
  void read_line(){ auto self=shared_from_this(); boost::asio::async_read_until(socket_, buffer_, '\n',[self](const boost::system::error_code& e,std::size_t){ if(e) return; std::istream in(&self->buffer_); std::string line; std::getline(in,line); if(!line.empty()&&line.back()=='\r') line.pop_back(); if(!line.empty()) self->handler_(std::move(line)); self->read_line();}); }
  boost::asio::ip::tcp::socket socket_; boost::asio::streambuf buffer_; MessageHandler handler_;
}; }
TcpServer::TcpServer(boost::asio::io_context& io,std::uint16_t port,MessageHandler handler): acceptor_(io,{boost::asio::ip::tcp::v4(),port}),handler_(std::move(handler)){ acceptor_.set_option(boost::asio::socket_base::reuse_address(true)); }
void TcpServer::start(){ do_accept(); }
void TcpServer::stop(){ boost::system::error_code ignored; acceptor_.close(ignored); }
void TcpServer::do_accept(){ acceptor_.async_accept([this](const boost::system::error_code& e,boost::asio::ip::tcp::socket socket){ if(!e) std::make_shared<TcpSession>(std::move(socket),handler_)->start(); else if(e!=boost::asio::error::operation_aborted) std::cerr<<"TCP accept error: "<<e.message()<<'\n'; if(acceptor_.is_open()) do_accept(); }); }
}  // namespace telemetry::network
