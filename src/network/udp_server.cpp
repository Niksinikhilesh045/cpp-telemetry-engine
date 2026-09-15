#include <utility>
#include "telemetry/network/udp_server.hpp"
#include <iostream>
namespace telemetry::network {
UdpServer::UdpServer(boost::asio::io_context& io,std::uint16_t port,MessageHandler handler):socket_(io,{boost::asio::ip::udp::v4(),port}),handler_(std::move(handler)){}
void UdpServer::start(){ receive_next(); }
void UdpServer::stop(){ boost::system::error_code ignored; socket_.close(ignored); }
void UdpServer::receive_next(){ socket_.async_receive_from(boost::asio::buffer(buffer_),remote_endpoint_,[this](const boost::system::error_code& e,std::size_t n){ if(!e){ std::string p(buffer_.data(),n); if(!p.empty()&&p.back()=='\n') p.pop_back(); if(!p.empty()&&p.back()=='\r') p.pop_back(); if(!p.empty()) handler_(std::move(p)); } else if(e!=boost::asio::error::operation_aborted) std::cerr<<"UDP receive error: "<<e.message()<<'\n'; if(socket_.is_open()) receive_next(); }); }
}  // namespace telemetry::network
