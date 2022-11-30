#include "types.hpp"
#include <di.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace di;

template <typename T>
struct MaybeLazyHolder {
    template <class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template <class... Ts>
    overloaded(Ts...)->overloaded<Ts...>;
    using ptr_t     = std::shared_ptr<T>;
    using factory_t = std::function<ptr_t()>;
    using variant_t = std::variant<ptr_t, factory_t>;

    variant_t data_;

    template <typename Fn>
    MaybeLazyHolder(Fn factory)
        : data_{ factory } {
    }

    MaybeLazyHolder(ptr_t ptr)
        : data_{ ptr } {
    }

    ptr_t get() {
        return std::visit(overloaded{
                              [](ptr_t ptr) {
                                  return ptr;
                              },
                              [this](factory_t factory) mutable {
                                  // todo: maybe lock here or smth
                                  return data_.template emplace<ptr_t>(factory());
                              } },
            data_);
    }

    ptr_t operator->() {
        return get();
    }

    ptr_t operator*() {
        return get();
    }
};

template <typename... Types>
using MaybeLazyServices = Selection<MaybeLazyHolder, Types...>;

TEST(MaybeLazyServicesTest, LazyOnly) {
    auto a_created = false;
    auto b_created = false;

    auto services = MaybeLazyServices<A, B>{
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

TEST(MaybeLazyServicesTest, LazyAndEager) {
    auto a_created = false;
    auto services  = MaybeLazyServices<A, B>{
        [&a_created] {
            a_created = true;
            return std::make_shared<A>();
        },                    // lazy
        std::make_shared<B>() // eager
    };

    EXPECT_FALSE(a_created);
    auto a = services.get<A>();
    EXPECT_FALSE(a_created);

    [[maybe_unused]] auto _ = a->value; // lazy
    EXPECT_TRUE(a_created);
}
