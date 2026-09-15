#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <future>
#include "telemetry/thread_pool.hpp"
TEST(ThreadPoolTest, ExecutesSubmittedTasks){ telemetry::ThreadPool pool(3); std::atomic<int> counter{0}; std::promise<void> done; auto future=done.get_future(); for(int i=0;i<20;++i) ASSERT_TRUE(pool.submit([&]{if(counter.fetch_add(1)+1==20) done.set_value();})); ASSERT_EQ(future.wait_for(std::chrono::seconds(2)),std::future_status::ready); EXPECT_EQ(counter.load(),20); pool.shutdown(); }
