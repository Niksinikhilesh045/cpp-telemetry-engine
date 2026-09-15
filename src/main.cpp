#include "telemetry/engine.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
namespace { std::uint16_t port(const std::string& v){auto p=std::stoul(v);if(p==0||p>65535)throw std::out_of_range("invalid port");return static_cast<std::uint16_t>(p);} }
int main(int argc,char* argv[]){try{telemetry::TelemetryEngineConfig c;auto hw=std::thread::hardware_concurrency();c.worker_threads=hw?hw:4;for(int i=1;i<argc;++i){std::string a=argv[i];auto next=[&](){if(i+1>=argc)throw std::invalid_argument("missing value");return std::string(argv[++i]);};if(a=="--tcp-port")c.tcp_port=port(next());else if(a=="--udp-port")c.udp_port=port(next());else if(a=="--workers")c.worker_threads=std::stoul(next());else if(a=="--alerts-file")c.alerts_file=next();else if(a=="--help"||a=="-h"){std::cout<<"telemetry_engine [--tcp-port P] [--udp-port P] [--workers N] [--alerts-file PATH]\n";return 0;}else throw std::invalid_argument("unknown option: "+a);}telemetry::TelemetryEngine e(c);e.add_rule(std::make_unique<telemetry::ThresholdRule>("temperature",90.0,telemetry::Comparison::GreaterThan,telemetry::Severity::Critical));e.add_rule(std::make_unique<telemetry::ThresholdRule>("pressure",100.0,telemetry::Comparison::LessThan,telemetry::Severity::Warning));e.add_rule(std::make_unique<telemetry::ThresholdRule>("rpm",4000.0,telemetry::Comparison::GreaterThan,telemetry::Severity::Warning));e.run();return 0;}catch(const std::exception& e){std::cerr<<"Fatal error: "<<e.what()<<'\n';return 1;}}
