#include <utility>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {
struct Options { std::string protocol{"tcp"}; std::string host{"127.0.0.1"}; std::uint16_t port{9000}; int devices{3}; int messages{10}; int interval_ms{100}; };
std::uint16_t parse_port(const std::string& value){ auto parsed=std::stoul(value); if(parsed==0||parsed>65535) throw std::out_of_range("port must be between 1 and 65535"); return static_cast<std::uint16_t>(parsed); }
Options parse_options(int argc,char* argv[]){ Options o; for(int i=1;i<argc;++i){ std::string a=argv[i]; auto next=[&](){ if(i+1>=argc) throw std::invalid_argument("missing value for "+a); return std::string(argv[++i]); }; if(a=="--protocol") o.protocol=next(); else if(a=="--host") o.host=next(); else if(a=="--port") o.port=parse_port(next()); else if(a=="--devices") o.devices=std::stoi(next()); else if(a=="--messages") o.messages=std::stoi(next()); else if(a=="--interval-ms") o.interval_ms=std::stoi(next()); else if(a=="--help"||a=="-h"){ std::cout<<"device_simulator --protocol tcp|udp --host HOST --port PORT [--devices N] [--messages N] [--interval-ms N]\n"; std::exit(0);} else throw std::invalid_argument("unknown option: "+a);} if(o.protocol!="tcp"&&o.protocol!="udp") throw std::invalid_argument("protocol must be tcp or udp"); if(o.devices<=0||o.messages<=0||o.interval_ms<0) throw std::invalid_argument("invalid simulation options"); return o; }
std::string make_message(int device,std::mt19937& gen){ std::uniform_real_distribution<double> t(70.0,105.0),p(92.0,112.0); std::uniform_int_distribution<int> rpm(2500,4800); auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); std::ostringstream out; out<<std::fixed<<std::setprecision(2)<<"device=DEVICE_"<<std::setw(3)<<std::setfill('0')<<device<<";temperature="<<t(gen)<<";pressure="<<p(gen)<<";rpm="<<rpm(gen)<<";timestamp="<<now; return out.str(); }
void run_tcp(const Options& o){ boost::asio::io_context io; boost::asio::ip::tcp::resolver resolver(io); boost::asio::ip::tcp::socket socket(io); boost::asio::connect(socket,resolver.resolve(o.host,std::to_string(o.port))); std::mt19937 gen(std::random_device{}()); for(int i=0;i<o.messages;++i){ auto payload=make_message((i%o.devices)+1,gen)+"\n"; boost::asio::write(socket,boost::asio::buffer(payload)); std::cout<<"TCP -> "<<payload; std::this_thread::sleep_for(std::chrono::milliseconds(o.interval_ms)); }}
void run_udp(const Options& o){ boost::asio::io_context io; boost::asio::ip::udp::resolver resolver(io); boost::asio::ip::udp::socket socket(io); socket.open(boost::asio::ip::udp::v4()); auto endpoints=resolver.resolve(o.host,std::to_string(o.port)); auto endpoint=*endpoints.begin(); std::mt19937 gen(std::random_device{}()); for(int i=0;i<o.messages;++i){ auto payload=make_message((i%o.devices)+1,gen); socket.send_to(boost::asio::buffer(payload),endpoint.endpoint()); std::cout<<"UDP -> "<<payload<<'\n'; std::this_thread::sleep_for(std::chrono::milliseconds(o.interval_ms)); }}
}
int main(int argc,char* argv[]){ try{ auto o=parse_options(argc,argv); if(o.protocol=="tcp") run_tcp(o); else run_udp(o); return 0;} catch(const std::exception& e){ std::cerr<<"Simulator error: "<<e.what()<<'\n'; return 1;} }
