#include <gtest/gtest.h>
#include <thread>
#include "telemetry/blocking_queue.hpp"
TEST(BlockingQueueTest, TransfersValueAcrossThreads){ telemetry::BlockingQueue<int> q; std::thread producer([&]{EXPECT_TRUE(q.push(42));}); auto v=q.wait_pop(); producer.join(); ASSERT_TRUE(v); EXPECT_EQ(*v,42); }
TEST(BlockingQueueTest, CloseUnblocksConsumer){ telemetry::BlockingQueue<int> q; q.close(); EXPECT_FALSE(q.wait_pop()); EXPECT_FALSE(q.push(7)); }
