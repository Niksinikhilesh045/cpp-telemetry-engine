#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "telemetry/event.hpp"

namespace telemetry {

struct ParseResult {
    std::optional<TelemetryEvent> event;
    std::string error;

    explicit operator bool() const noexcept { return event.has_value(); }
};

class TelemetryParser {
   public:
    [[nodiscard]] ParseResult parse(std::string_view payload, Transport transport) const;
};

}  // namespace telemetry
