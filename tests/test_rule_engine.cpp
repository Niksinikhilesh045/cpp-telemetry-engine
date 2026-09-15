#include <gtest/gtest.h>
#include <memory>
#include "telemetry/rule.hpp"
TEST(RuleEngineTest, EmitsAlertsForTriggeredThresholds){ telemetry::RuleEngine e; e.add_rule(std::make_unique<telemetry::ThresholdRule>("temperature",90,telemetry::Comparison::GreaterThan,telemetry::Severity::Critical)); e.add_rule(std::make_unique<telemetry::ThresholdRule>("pressure",100,telemetry::Comparison::LessThan,telemetry::Severity::Warning)); telemetry::TelemetryEvent event{"DEVICE_001",95,98,3500,1700000000000,telemetry::Transport::Udp}; auto alerts=e.evaluate(event); ASSERT_EQ(alerts.size(),2U); }
TEST(RuleEngineTest, EmitsNothingWhenHealthy){ telemetry::RuleEngine e; e.add_rule(std::make_unique<telemetry::ThresholdRule>("rpm",4000,telemetry::Comparison::GreaterThan,telemetry::Severity::Warning)); telemetry::TelemetryEvent event{"DEVICE_002",80,105,3200,1700000000000,telemetry::Transport::Tcp}; EXPECT_TRUE(e.evaluate(event).empty()); }
