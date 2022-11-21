#include <di.hpp>

#include <benchmark/benchmark.h>
#include <string>

struct A {
    int value = 1234;
};
struct B {
    bool value = false;
};
struct C {
    std::string value = "Unchanged";
};
struct D {
    float value = 0.42f;
};

struct User {
    using services_t = di::Services<A, B, const C>;
    User(services_t services)
        : services_{ services } {}
    services_t services_;
};

struct RefWrapperUser {
    RefWrapperUser(A &a, B &b, const C &c)
        : a_{ a }
        , b_{ b }
        , c_{ c } {}

    std::reference_wrapper<A> a_;
    std::reference_wrapper<B> b_;
    std::reference_wrapper<const C> c_;
};

struct PtrUser {
    PtrUser(std::shared_ptr<A> a, std::shared_ptr<B> b, std::shared_ptr<const C> c)
        : a_{ a }
        , b_{ b }
        , c_{ c } {}

    std::shared_ptr<A> a_;
    std::shared_ptr<B> b_;
    std::shared_ptr<const C> c_;
};

static void Benchmark_ServiceCreation(benchmark::State &state) {
    for(auto _ : state)
        [[maybe_unused]] auto tmp = di::Services<A, B, C>();
}
BENCHMARK(Benchmark_ServiceCreation);

static void Benchmark_SharedPtrCreation(benchmark::State &state) {
    for(auto _ : state) {
        std::make_shared<A>();
        std::make_shared<B>();
        std::make_shared<C>();
    }
}
BENCHMARK(Benchmark_SharedPtrCreation);

static void Benchmark_ServicePassing(benchmark::State &state) {
    auto services = di::Services<A, B, C, D>{};
    std::vector<User> vec;
    for(auto _ : state)
        vec.emplace_back(services);
}
BENCHMARK(Benchmark_ServicePassing);

static void Benchmark_RefWrapperPassing(benchmark::State &state) {
    auto a = A{};
    auto b = B{};
    auto c = C{};
    std::vector<RefWrapperUser> vec;
    for(auto _ : state)
        vec.emplace_back(a, b, c);
}
BENCHMARK(Benchmark_RefWrapperPassing);

static void Benchmark_SharedPtrPassing(benchmark::State &state) {
    auto a = std::make_shared<A>();
    auto b = std::make_shared<B>();
    auto c = std::make_shared<C>();
    std::vector<PtrUser> vec;
    for(auto _ : state)
        vec.emplace_back(a, b, c);
}
BENCHMARK(Benchmark_SharedPtrPassing);

static void Benchmark_ServiceExtending(benchmark::State &state) {
    auto services = di::Services<A, B, C>{};
    for(auto _ : state)
        [[maybe_unused]] auto extended = di::extend(services, std::make_shared<D>());
}
BENCHMARK(Benchmark_ServiceExtending);

static void Benchmark_ServiceCombining(benchmark::State &state) {
    auto s1 = di::Services<A, B>{};
    auto s2 = di::Services<C, D>{};
    for(auto _ : state)
        [[maybe_unused]] auto combined = di::combine(s1, s2);
}
BENCHMARK(Benchmark_ServiceCombining);