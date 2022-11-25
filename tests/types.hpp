#pragma once

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

struct Config {
    int severity = 3;
};