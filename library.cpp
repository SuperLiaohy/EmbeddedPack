#include <array>

#include "library.h"
#include "Components/RingBuffer.h"
#include <iostream>
#include <thread>
#include <random>

#include "Components/Detect.h"
#include "Components/Manager.h"
#include "Math/Matrix/Matrix.hpp"
#include "Math/Quaternion/Quaternion.hpp"
#include "Components/Format.h"

// ultimate_fair_benchmark.cpp
#include <iostream>
#include <iomanip>
#include <chrono>
#include <format>
#include <fmt/format.h>
#include <cstdio>

constexpr int N = 20'000'000;
char buf[256];
volatile uint64_t black_hole = 0;  // 绝对防优化

// 1. sprintf
void test_sprintf() {
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(1000, 9999);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        int number = distribution(gen);

        int len = std::sprintf(buf, "user %d score %d rank %.5f", number, number+1, 1.214213f);
        black_hole += len;
    }
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "sprintf       : " << std::fixed << std::setprecision(3) << ms << " ms\n";
}

// 2. std::format_to (C++20 官方直接写 buf)
void test_std_format_to() {
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(1000, 9999);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        int number = distribution(gen);
        auto it = std::format_to(buf, "user {} score {} rank {}", number, number+1, 100-number);
        black_hole += std::bit_cast<uint64_t>(it);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "std::format_to: " << ms << " ms\n";
}

// 3. fmt::format_to
void test_fmt_format_to() {
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(1000, 9999);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        int number = distribution(gen);
        auto it = fmt::format_to(buf, "user {} score {} rank {}", number, number+1, 100-number);
        black_hole += std::bit_cast<uint64_t>(it);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "fmt::format_to: " << ms << " ms\n";
}

void test_own_fmt_to() {
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(1000, 9999);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        int number = distribution(gen);
        // auto len = EP::Component::format<EP::Component::Str{"user {} score {} rank {}"}>(buf, number, number+1, 100-number);
        auto len = EP::Component::format<EP::Component::Str{"user {} score {} rank {.5}"}>(buf, number, number+1, 1.214213f);
        black_hole += len;
    }
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "own_fmt_to    : " << ms << " ms\n";
}

int main() {
    std::cout << "2000万次！\n\n";

    test_sprintf();
    // test_std_format_to();
    // test_fmt_format_to();
    test_own_fmt_to();

    std::cout << "\n示例输出: " << buf << "\n";
}