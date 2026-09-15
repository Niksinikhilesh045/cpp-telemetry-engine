#include "telemetry/rule.hpp"

#include <stdexcept>
#include <utility>

namespace telemetry {

ThresholdRule::ThresholdRule(std::string metric, double threshold, Comparison comparison,
                             Severity severity)
    : metric_(std::move(metric)),
      threshold_(threshold),
      comparison_(comparison),
      severity_(severity) {
    if (metric_ != "temperature" && metric_ != "pressure" && metric_ != "rpm") {
        throw std::invalid_argument("unsupported telemetry metric: " + metric_);
    }
}

std::optional<double> ThresholdRule::metric_value(const TelemetryEvent& event) const {
    if (metric_ == "temperature") return event.temperature;
    if (metric_ == "pressure") return event.pressure;
    if (metric_ == "rpm") return static_cast<double>(event.rpm);
    return std::nullopt;
}

std::optional<Alert> ThresholdRule::evaluate(const TelemetryEvent& event) const {
    const auto value = metric_value(event);
    if (!value) return std::nullopt;
    const bool triggered = comparison_ == Comparison::GreaterThan ? *value > threshold_ : *value < threshold_;
    if (!triggered) return std::nullopt;
    return Alert{event.device_id, metric_, *value, threshold_, comparison_, severity_, event.timestamp_ms,
                 event.transport};
}

void RuleEngine::add_rule(std::unique_ptr<IRule> rule) {
    if (!rule) throw std::invalid_argument("rule must not be null");
    rules_.push_back(std::move(rule));
}

std::vector<Alert> RuleEngine::evaluate(const TelemetryEvent& event) const {
    std::vector<Alert> alerts;
    alerts.reserve(rules_.size());
    for (const auto& rule : rules_) {
        if (auto alert = rule->evaluate(event)) alerts.push_back(std::move(*alert));
    }
    return alerts;
}

}  // namespace telemetry
