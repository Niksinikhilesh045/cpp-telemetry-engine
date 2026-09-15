#include <gtest/gtest.h>
#include "telemetry/parser.hpp"
TEST(TelemetryParserTest, ParsesValidPayload){ telemetry::TelemetryParser p; auto r=p.parse("device=DEVICE_001;temperature=91.5;pressure=99.2;rpm=4100;timestamp=1700000000000",telemetry::Transport::Tcp); ASSERT_TRUE(r); EXPECT_EQ(r.event->device_id,"DEVICE_001"); EXPECT_DOUBLE_EQ(r.event->temperature,91.5); EXPECT_EQ(r.event->rpm,4100); }
TEST(TelemetryParserTest, RejectsMissingField){ telemetry::TelemetryParser p; auto r=p.parse("device=A;temperature=80;pressure=100;rpm=3000",telemetry::Transport::Udp); EXPECT_FALSE(r); }
TEST(TelemetryParserTest, RejectsDuplicateField){ telemetry::TelemetryParser p; auto r=p.parse("device=A;device=B;temperature=80;pressure=105;rpm=3000;timestamp=1700000000000",telemetry::Transport::Tcp); EXPECT_FALSE(r); }
