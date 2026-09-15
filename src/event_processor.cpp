#include "telemetry/event_processor.hpp"
namespace telemetry {
std::size_t EventProcessor::process(const TelemetryEvent& event) const {
    const auto alerts = rules_.evaluate(event);
    for (const auto& alert : alerts) sink_.publish(alert);
    return alerts.size();
}
}  // namespace telemetry
