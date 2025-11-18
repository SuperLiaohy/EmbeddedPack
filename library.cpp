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

auto get_systime() -> uint32_t {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    uint32_t systime = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    return systime;
};

struct empty_struct {
    static int a;
    static int b;
    static void print() {
        std::cout << "static a: "<< a << std::endl;
        std::cout << "empty struct" << std::endl;
    }
    ~empty_struct() {
        std::cout << "empty struct is closed" << std::endl;
    }
};
int empty_struct::a = 1;
int empty_struct::b = 1;
struct real_struct : empty_struct{
    int value;
    real_struct(int v) : value(v) {}
    void print() {
        std::cout << "real_struct " << value << std::endl;
    }
    ~real_struct() {
        std::cout << "real_struct is closed" << std::endl;
    }
};



int main() {
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();

    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(1000, 9999);
    char buffer[256]{};

    auto start1_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < UINT16_MAX*1000; ++i) {
        int number = distribution(gen);
        sprintf(buffer, "123456 %d 789 ", number);

        // EP::Component::format<EP::Component::Str{"123456 {} 789 {}"}>(buffer,INT32_MIN,INTMAX_MAX);
        // std::cout << buffer << std::endl;
    }
    auto end1_time = std::chrono::high_resolution_clock::now();

    std::cout << "sprintf fmt:" << std::chrono::duration_cast<std::chrono::microseconds>((end1_time-start1_time)).count()/1000.f << std::endl;



    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < UINT16_MAX*1000; ++i) {
        int number = distribution(gen);

        EP::Component::format<EP::Component::Str{"123456 {} 789 "}>(buffer,number);
        // std::cout << buffer << std::endl;
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "own fmt: " << std::chrono::duration_cast<std::chrono::microseconds>((end_time-start_time)).count()/1000.f << std::endl;




    // std::cout << "normalized(): " << ColVec<3>{{2,2,0}}.normalized() << std::endl;
    // UnitQuat q{120*std::numbers::pi/180,ColVec<3>{{2,2,0}}.normalized()};
    // UnitQuat q1(q.normalized());
    //
    // std::cout << q.inv() << std::endl;
    // std::cout << q1 << std::endl;
    //
    // std::cout << "empty_struct size: " << sizeof(empty_struct) << std::endl;
    // std::cout << "real_struct size: " << sizeof(real_struct) << std::endl;
    // empty_struct tool;
    // tool.print();
    // std::cout << "empty_struct size: " << sizeof(tool) << std::endl;
    // real_struct real(2);
    // real.print();
    // empty_struct::print();
    // std::cout << "real_struct size: " << sizeof(real_struct) << std::endl;
    //
    //
    // ColVec<3> v1({1, 2, 3});
    // ColVec<3> v2({1, 2, 3});
    // RowVec<3> a1({3, 2, 1});
    // RowVec<3> a2({3, 2, 1});
    // std::cout << "v1: " << v1 << std::endl;
    // std::cout << "v2: " << v2 << std::endl;
    // std::cout << "v1 * v2: " << v1 * v2 << std::endl;
    // std::cout << "v1 ^ v2: " << (v1 ^ v2) << std::endl;
    // std::cout << "a1: " << a1 << std::endl;
    // std::cout << "a2: " << a2 << std::endl;
    // std::cout << "a1 * v2: " << a1 * v2 << std::endl;
    // std::cout << "a1 ^ a2: " << (a1 ^ a2) << std::endl;
    // std::cout << "test a1: " << a1.transpose() << std::endl;
    // std::cout << "a1 transpose: " << ~a1 * a2 << std::endl;
    // std::cout << "3 * a1: " << 3 * a1 << std::endl;
    // std::cout << "3 * a1: " << a2 * 3 << std::endl;
    //
    // // float map[3][3] =
    //
    // Matrix<3,3> matrix({
    //     {1,2,3},
    //     {0,1,0},
    //     {0,0,1}
    // });
    // std::cout << "matrix: " << matrix << std::endl;
    // std::cout << "matrix transpose: " << matrix.transpose() << std::endl;
    // std::cout << "matrix transpose: " << matrix * matrix.transpose() << std::endl;
    // std::cout << "matrix inv: " << matrix.inv() * matrix << std::endl;
    // std::cout << "matrix inv: " << decltype(matrix)::eyes().inv() << std::endl;
    //
    // // matrix = nullptr;
    // if (matrix==nullptr) {
    //     std::cout << "matrix is null" << std::endl;
    // } else {
    //     std::cout << "matrix is not null" << std::endl;
    // }
    // auto& detectManager = EP::Component::Manager<EP::Component::Detect<get_systime>, 10>::instance();
    // // EP::Component::Detect::State s = EP::Component::Detect::State::LOSE;
    // auto detectPtr0 = detectManager.make_managed(1000,
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Working" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Missing" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Recover" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Lose" << std::endl;}
    //     );
    //
    // auto detectPtr1 = detectManager.make_managed(1000,
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Working" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Missing" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Recover" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Lose" << std::endl;}
    //     );
    //
    // {
    //     auto detectPtr2 = detectManager.make_managed(1000,
    //        [](void* parament){std::cout << *static_cast<int*>(parament) << " Working" << std::endl;},
    //        [](void* parament){std::cout << *static_cast<int*>(parament) << " Missing" << std::endl;},
    //        [](void* parament){std::cout << *static_cast<int*>(parament) << " Recover" << std::endl;},
    //        [](void* parament){std::cout << *static_cast<int*>(parament) << " Lose" << std::endl;}
    //        );
    // }
    //
    // auto detectPtr3 = detectManager.make_managed(1000,
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Working" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Missing" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Recover" << std::endl;},
    //     [](void* parament){std::cout << *static_cast<int*>(parament) << " Lose" << std::endl;}
    //     );
    //
    // auto detectPtr4 = std::move(detectPtr3);
    //
    // std::thread detectThread([&detectManager]() {
    //     while (true) {
    //         int index = 0;
    //         std::cout << "system_time: " << get_systime() << std::endl;
    //         for (auto & detect : detectManager) {
    //             ++index;
    //             switch (detect.detect()) {
    //                 case EP::Component::detectDep::LOSE:
    //                     detect.lose(&index);
    //                     break;
    //                 case EP::Component::detectDep::MISSING:
    //                     detect.missing(&index);
    //                     break;
    //                 case EP::Component::detectDep::WORKING:
    //                     detect.working(&index);
    //                     break;
    //                 case EP::Component::detectDep::RECOVER:
    //                     detect.recover(&index);
    //                     break;
    //             }
    //         }
    //         std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     }
    // });
    //
    // while (true) {
    //     std::cout << "******************************" <<std::endl;
    //     for (auto & detect : detectManager) {
    //         detect.update();
    //     }
    //     std::cout << "******************************" <<std::endl;
    //
    //     std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    // }
    // detectThread.join();
    //
    // uint8_t str_buf[256]{};
    // std::cout << str_buf << std::endl;
    //
    // EP::Component::RingBuffer buffer(6);
    // std::cout << "size: " << buffer.size() << std::endl;
    // std::cout << "capacity: " << buffer.capacity() << std::endl;
    // buffer.push_force((4));
    // buffer.push_force((2));
    // buffer.push_force((1));
    // buffer.push_force((3));
    // buffer.push_force((5));
    // buffer.push_force((3));
    // buffer.push_force((1));
    // buffer.push_force((0));
    // buffer.pop();
    // std::array<uint8_t,3> test ={6,7,8};
    // buffer.write_data_force(test.data(), test.size());
    // std::cout << "size: " << buffer.size() << std::endl;
    // std::cout << "capacity: " << buffer.capacity() << std::endl;
    // auto v = buffer.get_valid_container();
    // for (auto& item: v) {
    //     std::cout << static_cast<uint32_t>(item) << std::endl;
    // }

}

// #include <iostream>
// #include <utility> // for std::forward, std::index_sequence
// #include <tuple>   // for std::tuple, std::tuple_element_t
//
// // 获取类型名称的辅助函数
// template <typename T>
// const char* type_name() {
// #ifdef _MSC_VER
//     return __FUNCSIG__;
// #else
//     return __PRETTY_FUNCTION__;
// #endif
// }
//
// // --- 内部实现 ---
//
// template <typename ArgTuple, typename IndexSequence>
// struct for_each_impl;
//
// template <typename ArgTuple, std::size_t... I>
// struct for_each_impl<ArgTuple, std::index_sequence<I...>> {
//     static void execute() {
//         (
//             []<typename T, std::size_t Index>() {
//                 std::cout << "  索引 " << Index << ": 类型是 " << type_name<T>() << std::endl;
//             }.template operator()<std::tuple_element_t<I, ArgTuple>, I>()
//             , ...
//         );
//     }
// };
//
// template <typename... Args>
// void for_each() {
//     using ArgTuple = std::tuple<Args...>;
//     using Indices = std::make_index_sequence<sizeof...(Args)>;
//     for_each_impl<ArgTuple, Indices>::execute();
// }
//
//
// int main() {
//     std::cout << "版本5：终极接口 + 内部纯类型分发\n";
//
//     // 调用方式和版本4完全一样，保持了最理想的形式。
//     for_each<int, double, const char*>();
//
//     return 0;
// }
