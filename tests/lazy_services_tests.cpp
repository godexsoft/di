#include "types.hpp"
#include <di.hpp>

#include <atomic>
#include <gtest/gtest.h>
#include <string>
#include <thread>

using namespace di;

TEST(LazyServicesTest, LazyOnly) {
    auto a_created = false;
    auto b_created = false;

    auto services = LazyServices<A, B>{
        [&a_created] {
            a_created = true;
            return std::make_shared<A>();
        },
        [&b_created] {
            b_created = true;
            return std::make_shared<B>();
        }
    };

    EXPECT_FALSE(a_created);
    EXPECT_FALSE(b_created);

    auto a = services.get<A>();
    EXPECT_FALSE(a_created);

    [[maybe_unused]] auto _ = a->value; // lazy
    EXPECT_TRUE(a_created);

    auto b = services.get<B>();
    EXPECT_FALSE(b_created);
    [[maybe_unused]] auto __ = b->value;
    EXPECT_TRUE(b_created);
}

TEST(LazyServicesTest, LazyAndEager) {
    auto a_create_count = 0;
    auto services       = LazyServices<A, B>{
        [&a_create_count] {
            ++a_create_count;
            return std::make_shared<A>();
        },                    // lazy
        std::make_shared<B>() // eager
    };

    EXPECT_EQ(a_create_count, 0);
    auto a = services.get<A>();
    EXPECT_EQ(a_create_count, 0);

    [[maybe_unused]] auto _ = a->value; // lazy
    EXPECT_EQ(a_create_count, 1);
    [[maybe_unused]] auto _2 = a->value;
    EXPECT_EQ(a_create_count, 1);
    [[maybe_unused]] auto _3 = a.get();
    EXPECT_EQ(a_create_count, 1);

    [[maybe_unused]] auto aa = services.get<A>(); // another
    EXPECT_EQ(a_create_count, 1);
    [[maybe_unused]] auto _21 = aa->value;
    EXPECT_EQ(a_create_count, 1);
    [[maybe_unused]] auto _31 = aa.get();
    EXPECT_EQ(a_create_count, 1);
}

TEST(LazyServicesTest, CreatedOnlyOnce) {
    auto a_created = false;
    auto services  = LazyServices<A>{
        [&a_created] {
            a_created = true;
            return std::make_shared<A>();
        }
    };

    EXPECT_FALSE(a_created);
    auto a = services.get<A>(); // a is LazyHolder<A>
    EXPECT_FALSE(a_created);
    auto original = services.get<A>().get(); // explicitly load
    EXPECT_TRUE(a_created);
    EXPECT_EQ(original, a.get());
}

TEST(LazyServicesTest, MultiThreadLazyLoadsOnce) {
    auto a_create_count = 0;
    auto b_create_count = 0;
    auto services       = LazyServices<A, B>{
        [&a_create_count] {
            ++a_create_count;
            return std::make_shared<A>();
        },
        [&b_create_count] {
            ++b_create_count;
            return std::make_shared<B>();
        }
    };

    EXPECT_EQ(a_create_count, 0);
    std::atomic<std::size_t> calls = 0;

    auto fn = [&services, &calls] {
        ++calls;
        for(auto i = 0; i < 100; ++i) {
            [[maybe_unused]] auto a = services.get<A>().get();
            [[maybe_unused]] auto b = services.get<B>().get();
        }
    };

    auto threads      = std::vector<std::thread>{};
    auto thread_count = 64;

    for(auto i = 0; i < thread_count; ++i)
        threads.emplace_back(fn);
    for(auto &thread : threads)
        thread.join();

    EXPECT_EQ(a_create_count, 1);
    EXPECT_EQ(b_create_count, 1);
    EXPECT_EQ(calls, thread_count);
}
