#include "types.hpp"
#include <di.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace di;

TEST(DepsTest, CompileChecks) {
    A a;
    B const b;
    [[maybe_unused]] Deps<A, const B> valid1{ std::ref(a), std::cref(b) };
    [[maybe_unused]] Deps<A, const B> valid2{ a, b };        // without explicit ref/cref
    [[maybe_unused]] Deps<const A, const B> valid3 = valid1; // transform non-const into const
    static_assert(std::is_same_v<decltype(valid1.get<A>()), std::reference_wrapper<A>>);
    static_assert(std::is_same_v<decltype(valid3.get<A>()), std::reference_wrapper<const A>>);

    // [[maybe_unused]] Deps<C> invalid = valid1; - no C in valid1
    // [[maybe_unused]] Deps<const A, B> invalid = valid1; - can't bind non-const B
    // [[maybe_unused]] Deps<A, B> invalid{A{}, B{}}; - can only accept std::ref()/std::cref()
    // [[maybe_unused]] Deps<B, A, C> invalid; - can't default-instantiate refs

    // [[maybe_unused]] Deps<A, B, B> invalid; - duplicates
    // [[maybe_unused]] Deps<B, A, B> invalid; - duplicates
    // [[maybe_unused]] Deps<B, const B> invalid; - duplicates
}

TEST(DepsTest, Extending) {
    A a;
    B b;
    C c;
    const D d;
    Deps<A, B> deps{ a, b };
    Deps<A, B, C> test     = extend(deps, std::ref(c));
    Deps<A, const B, C> hm = test;

    // deps is <A, B>
    // auto invalid = extend(deps, std::ref(b)); - fails to add one more B dep

    auto d1                  = Deps<A, B>{ a, b };
    [[maybe_unused]] auto d2 = extend(d1, c);
    static_assert(std::is_same_v<decltype(d2), Deps<A, B, C>>);

    auto d3                  = Deps<const A, B>{ a, b }; // explicitly const A
    [[maybe_unused]] auto d4 = extend(d3, c, d);         // d is const already
    static_assert(std::is_same_v<decltype(d4), Deps<const A, B, C, const D>>);

    auto d5                  = Deps<A, B>{ a, b };
    [[maybe_unused]] auto d6 = extend(d5, static_cast<const C>(c), d); // promote to const
    static_assert(std::is_same_v<decltype(d6), Deps<A, B, const C, const D>>);
}

TEST(DepsTest, Combining) {
    A a;
    B b;
    C c;
    D d;
    Deps<A, B> ab{ std::ref(a), std::ref(b) };
    Deps<C, D> cd{ std::ref(c), std::ref(d) };
    auto combined = combine(ab, cd);
    static_assert(std::is_same_v<decltype(combined), Deps<A, B, C, D>>);

    Config cfg;
    auto comb2 = combine(ab, extend(cd, std::ref(cfg)));
    static_assert(std::is_same_v<decltype(comb2), Deps<A, B, C, D, Config>>);
    ASSERT_EQ(comb2.get<Config>().get().severity, 3);

    // auto invalid = combine(combined, ab); - can't add A and B because they already exist in combined
}

TEST(DepsTest, UsingStructuredBindings) {
    A a;
    B b;
    C c;
    Deps<A, B, C> abc{ std::ref(a), std::ref(b), std::ref(c) };

    auto a1 = abc.get<A>();
    static_assert(std::is_same_v<decltype(a1), std::reference_wrapper<A>>);

    auto [aa, bb] = abc.get<A, const B>();
    static_assert(std::is_same_v<decltype(aa), std::reference_wrapper<A>>);
    static_assert(std::is_same_v<decltype(bb), std::reference_wrapper<const B>>);
}