#pragma once

#include <cstddef>
#include "telemetry/alert_sink.hpp"
#include "telemetry/event.hpp"
#include "telemetry/rule.hpp"

namespace telemetry {
class EventProcessor {
   public:
    EventProcessor(const RuleEngine& rules, IAlertSink& sink) : rules_(rules), sink_(sink) {}
    [[nodiscard]] std::size_t process(const TelemetryEvent& event) const;
   private:
    const RuleEngine& rules_;
    IAlertSink& sink_;
};
}  // namespace telemetry
