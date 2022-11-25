#include <di.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace di;

class Base {
public:
    virtual ~Base()     = default;
    virtual void test() = 0;
};

class Derived : public Base {
public:
    virtual ~Derived() = default;
    virtual void test() override {}
};

class Mock : public Base {
public:
    bool wasCalled = false;

    virtual ~Mock() = default;
    virtual void test() override { wasCalled = true; }
};

TEST(MockTests, MockExample) {
    auto mock = std::make_shared<Mock>(); // Real services would have Derived instead
    ASSERT_FALSE(mock->wasCalled);

    Services<Base> services(mock);
    services.get<Base>()->test();
    ASSERT_TRUE(mock->wasCalled);
}