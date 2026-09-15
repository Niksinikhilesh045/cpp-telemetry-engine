#include "telemetry/engine.hpp"
#include <algorithm>
#include <csignal>
#include <iostream>
#include <utility>
namespace telemetry {
TelemetryEngine::TelemetryEngine(TelemetryEngineConfig c):config_(std::move(c)),signals_(io_context_,SIGINT,SIGTERM),workers_(std::max<std::size_t>(1,config_.worker_threads)),sink_(std::make_unique<AsyncAlertSink>(create_sink(config_.alerts_file))),processor_(std::make_unique<EventProcessor>(rules_,*sink_)){
 tcp_server_=std::make_unique<network::TcpServer>(io_context_,config_.tcp_port,[this](std::string p){handle_message(std::move(p),Transport::Tcp);});
 udp_server_=std::make_unique<network::UdpServer>(io_context_,config_.udp_port,[this](std::string p){handle_message(std::move(p),Transport::Udp);});
 signals_.async_wait([this](const boost::system::error_code& e,int sig){if(!e){std::cerr<<"Received signal "<<sig<<", stopping...\n";stop();}});
}
TelemetryEngine::~TelemetryEngine(){stop();}
void TelemetryEngine::add_rule(std::unique_ptr<IRule> r){rules_.add_rule(std::move(r));}
void TelemetryEngine::run(){tcp_server_->start();udp_server_->start();std::cout<<"Telemetry engine listening on TCP "<<config_.tcp_port<<" and UDP "<<config_.udp_port<<" with "<<workers_.thread_count()<<" workers\n";std::cout<<"Configured rules: "<<rules_.rule_count()<<'\n';io_context_.run();workers_.shutdown();sink_->stop();auto s=stats();std::cout<<"Engine stopped. received="<<s.received<<" parsed="<<s.parsed<<" parse_errors="<<s.parse_errors<<" alerts="<<s.alerts<<'\n';}
void TelemetryEngine::stop(){bool expected=false;if(!stopped_.compare_exchange_strong(expected,true))return;if(tcp_server_)tcp_server_->stop();if(udp_server_)udp_server_->stop();io_context_.stop();}
EngineStats TelemetryEngine::stats()const noexcept{return{received_.load(),parsed_.load(),parse_errors_.load(),alerts_.load()};}
void TelemetryEngine::handle_message(std::string p,Transport t){received_.fetch_add(1);workers_.submit([this,p=std::move(p),t](){auto r=parser_.parse(p,t);if(!r){parse_errors_.fetch_add(1);std::lock_guard<std::mutex> lock(log_mutex_);std::cerr<<"Rejected "<<to_string(t)<<" payload: "<<r.error<<'\n';return;}parsed_.fetch_add(1);alerts_.fetch_add(processor_->process(*r.event));});}
std::unique_ptr<IAlertSink> TelemetryEngine::create_sink(const std::string& f){if(f.empty())return std::make_unique<ConsoleAlertSink>();return std::make_unique<JsonFileAlertSink>(f);}
}  // namespace telemetry
