#pragma once

#include <cstdint>
#include <string>

namespace telemetry {

enum class Transport { Tcp, Udp };

inline const char* to_string(Transport transport) noexcept {
    return transport == Transport::Tcp ? "tcp" : "udp";
}

struct TelemetryEvent {
    std::string device_id;
    double temperature{0.0};
    double pressure{0.0};
    int rpm{0};
    std::int64_t timestamp_ms{0};
    Transport transport{Transport::Tcp};
};

}  // namespace telemetry
