#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "telemetry/event_processor.hpp"
namespace { class CapturingSink final: public telemetry::IAlertSink{ public: void publish(const telemetry::Alert& a) override{alerts.push_back(a);} std::vector<telemetry::Alert> alerts; }; }
TEST(EventProcessorTest, PublishesMatches){ telemetry::RuleEngine rules; rules.add_rule(std::make_unique<telemetry::ThresholdRule>("temperature",90,telemetry::Comparison::GreaterThan,telemetry::Severity::Critical)); CapturingSink sink; telemetry::EventProcessor p(rules,sink); telemetry::TelemetryEvent e{"DEVICE_007",99,105,3000,1700000000000,telemetry::Transport::Tcp}; EXPECT_EQ(p.process(e),1U); ASSERT_EQ(sink.alerts.size(),1U); }
