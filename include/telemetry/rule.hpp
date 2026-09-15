#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "telemetry/event.hpp"

namespace telemetry {

enum class Comparison { GreaterThan, LessThan };
enum class Severity { Warning, Critical };

inline const char* to_string(Comparison comparison) noexcept {
    return comparison == Comparison::GreaterThan ? ">" : "<";
}

inline const char* to_string(Severity severity) noexcept {
    return severity == Severity::Critical ? "critical" : "warning";
}

struct Alert {
    std::string device_id;
    std::string metric;
    double observed_value{0.0};
    double threshold{0.0};
    Comparison comparison{Comparison::GreaterThan};
    Severity severity{Severity::Warning};
    std::int64_t timestamp_ms{0};
    Transport transport{Transport::Tcp};
};

class IRule {
   public:
    virtual ~IRule() = default;
    [[nodiscard]] virtual std::optional<Alert> evaluate(const TelemetryEvent& event) const = 0;
};

class ThresholdRule final : public IRule {
   public:
    ThresholdRule(std::string metric, double threshold, Comparison comparison, Severity severity);

    [[nodiscard]] std::optional<Alert> evaluate(const TelemetryEvent& event) const override;

   private:
    [[nodiscard]] std::optional<double> metric_value(const TelemetryEvent& event) const;

    std::string metric_;
    double threshold_;
    Comparison comparison_;
    Severity severity_;
};

class RuleEngine {
   public:
    void add_rule(std::unique_ptr<IRule> rule);
    [[nodiscard]] std::vector<Alert> evaluate(const TelemetryEvent& event) const;
    [[nodiscard]] std::size_t rule_count() const noexcept { return rules_.size(); }

   private:
    std::vector<std::unique_ptr<IRule>> rules_;
};

}  // namespace telemetry
