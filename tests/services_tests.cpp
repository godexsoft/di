#include "types.hpp"
#include <di.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace di;

TEST(ServicesTest, CompileChecks) {
    [[maybe_unused]] Services<A, B> valid1;
    [[maybe_unused]] Services<B, A, C> valid2;
    [[maybe_unused]] Services<const A, const B> valid3; // only const get possible

    // Services<const A, B> s = valid3; // invalid - can't bind non-const B
    // [[maybe_unused]] Services<A, B, B> invalid - duplicates
    // [[maybe_unused]] Services<B, A, B> invalid - duplicates
    // [[maybe_unused]] Services<B, const B> invalid - duplicates
}

TEST(ServicesTest, ConstUsage) {
    Services<A, B> ab;
    Services<B, A> ba   = ab;
    Services<const B> b = ba;
    auto ncb            = b.get<B>(); // implicit const get
    static_assert(std::is_same_v<decltype(ncb), std::shared_ptr<const B>>);

    auto cb = ab.get<const B>(); // explicit const get
    static_assert(std::is_same_v<decltype(cb), std::shared_ptr<const B>>);

    Services<Config, const B, A, C> top_level;
    top_level.get<Config>()->severity = 4; // can write

    Services<const Config, A> a_user = top_level; // severity should be 4 here too
    ASSERT_EQ(a_user.get<Config>()->severity, 4); // can only read
    a_user.get<A>()->value = 420;                 // can mutate A
    ASSERT_EQ(a_user.get<A>()->value, 420);
}

TEST(ServicesTest, Extending) {
    Services<A, B> services;
    Services<A, B, C> test     = extend(services, std::make_shared<C>());
    Services<A, const B, C> hm = test;

    Services<A, const B, C> test3 = extend(services, std::make_shared<C>());
    auto test2                    = extend(services, std::make_shared<C>());

    auto ext  = extend(services, test.get<C>());         // *1* inject from existing service
    auto ext2 = extend(services, std::make_shared<C>()); // *2* inject from new
    static_assert(std::is_same_v<decltype(ext), Services<A, B, C>>);

    test.get<C>()->value = "Changed";
    ASSERT_STREQ(test.get<C>()->value.c_str(), "Changed");
    ASSERT_STREQ(ext.get<C>()->value.c_str(), "Changed");
    ASSERT_STREQ(ext2.get<C>()->value.c_str(), "Unchanged");

    // services is <A, B>
    // auto fail1 = extend(services, std::make_shared<B>()); // fails to add one more B service
}

TEST(ServicesTest, Combining) {
    Services<A, B> ab;
    Services<C, D> cd;
    auto combined = combine(ab, cd);
    static_assert(std::is_same_v<decltype(combined), Services<A, B, C, D>>);

    auto comb2 = combine(ab, extend(cd, std::make_shared<Config>()));
    static_assert(std::is_same_v<decltype(comb2), Services<A, B, C, D, Config>>);
    ASSERT_EQ(comb2.get<Config>()->severity, 3);

    auto comb3 = combine(ab, cd, Services<int>{ std::make_shared<int>(123) });
    static_assert(std::is_same_v<decltype(comb3), Services<A, B, C, D, int>>);

    auto comb4 = combine(ab, cd, Services<int>{ std::make_shared<int>(123) }, Services<double>{ std::make_shared<double>(0.42) });
    static_assert(std::is_same_v<decltype(comb4), Services<A, B, C, D, int, double>>);

    auto comb5 = combine(ab, cd,
        Services<int>{ std::make_shared<int>(123) },
        Services<double>{ std::make_shared<double>(0.42) },
        Services<std::string>{ std::make_shared<std::string>("Hello") });
    static_assert(std::is_same_v<decltype(comb5), Services<A, B, C, D, int, double, std::string>>);

    // auto fail1 = combine(comb5, ab); // can't add A and B because they already exist in comb5
}

TEST(ServicesTest, UsingStructuredBindings) {
    Services<A, B, C> abc;
    auto [a, b] = abc.get<A, const B>();
    static_assert(std::is_same_v<decltype(a), std::shared_ptr<A>>);
    static_assert(std::is_same_v<decltype(b), std::shared_ptr<const B>>);
}

TEST(ServicesTest, FreeFunctionGet) {
    Services<A, B> ab;
    Services<B, A> ba   = ab;
    Services<const B> b = ba;
    auto ncb            = get<B>(b); // implicit const get
    static_assert(std::is_same_v<decltype(ncb), std::shared_ptr<const B>>);

    auto cb = get<const B>(ab); // explicit const get
    static_assert(std::is_same_v<decltype(cb), std::shared_ptr<const B>>);

    Services<Config, const B, A, C> top_level;
    get<Config>(top_level)->severity = 4; // can write

    Services<const Config, A> a_user = top_level; // severity should be 4 here too
    ASSERT_EQ(get<Config>(a_user)->severity, 4);  // can only read
    get<A>(a_user)->value = 420;                  // can mutate A
    ASSERT_EQ(get<A>(a_user)->value, 420);
}

TEST(ServicesTest, FreeFunctionTupleGet) {
    Services<A, B, C> abc;
    auto [a, b] = get<A, const B>(abc);
    static_assert(std::is_same_v<decltype(a), std::shared_ptr<A>>);
    static_assert(std::is_same_v<decltype(b), std::shared_ptr<const B>>);
}