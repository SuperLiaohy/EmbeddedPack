#include <array>

#include "library.h"
#include "Components/RingBuffer.h"
#include <iostream>
#include <thread>

#include "Components/Detect.h"
#include "Components/Manager.h"

EP::Component::Manager<EP::Component::Detect, 10>& detectManager() {
    static EP::Component::Manager<EP::Component::Detect, 10> manager;
    return manager;
};

auto get_systime() -> uint32_t {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    uint32_t systime = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    return systime;
};



int main() {


    // EP::Component::Detect::State s = EP::Component::Detect::State::LOSE;
    auto detectPtr = detectManager().make_managed(1000,
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Working" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Missing" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Recover" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Lose" << std::endl;}
        );

    auto detectPtr1 = detectManager().make_managed(1000,
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Working" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Missing" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Recover" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Lose" << std::endl;}
        );

    {
        auto detectPtr2 = detectManager().make_managed(1000,
           [](void* parament){std::cout << *static_cast<int*>(parament) << " Working" << std::endl;},
           [](void* parament){std::cout << *static_cast<int*>(parament) << " Missing" << std::endl;},
           [](void* parament){std::cout << *static_cast<int*>(parament) << " Recover" << std::endl;},
           [](void* parament){std::cout << *static_cast<int*>(parament) << " Lose" << std::endl;}
           );
    }

    auto detectPtr3 = detectManager().make_managed(1000,
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Working" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Missing" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Recover" << std::endl;},
        [](void* parament){std::cout << *static_cast<int*>(parament) << " Lose" << std::endl;}
        );

    std::thread detectThread([]() {
        while (true) {
            int index = 0;
            std::cout << "system_time: " << get_systime() << std::endl;
            for (auto & detect : detectManager()) {
                ++index;
                switch (detect.detect<get_systime>()) {
                    case EP::Component::Detect::LOSE:
                        detect.lose(&index);
                        break;
                    case EP::Component::Detect::MISSING:
                        detect.missing(&index);
                        break;
                    case EP::Component::Detect::WORKING:
                        detect.working(&index);
                        break;
                    case EP::Component::Detect::RECOVER:
                        detect.recover(&index);
                        break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    while (true) {
        std::cout << "******************************" <<std::endl;
        for (auto & detect : detectManager()) {
            detect.update<get_systime>();
        }
        std::cout << "******************************" <<std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
    detectThread.join();

    uint8_t str_buf[256]{};
    std::cout << str_buf << std::endl;

    EP::Component::RingBuffer buffer(6);
    std::cout << "size: " << buffer.size() << std::endl;
    std::cout << "capacity: " << buffer.capacity() << std::endl;
    buffer.push_force((4));
    buffer.push_force((2));
    buffer.push_force((1));
    buffer.push_force((3));
    buffer.push_force((5));
    buffer.push_force((3));
    buffer.push_force((1));
    buffer.push_force((0));
    buffer.pop();
    std::array<uint8_t,3> test ={6,7,8};
    buffer.write_data_force(test.data(), test.size());
    std::cout << "size: " << buffer.size() << std::endl;
    std::cout << "capacity: " << buffer.capacity() << std::endl;
    auto v = buffer.get_valid_container();
    for (auto& item: v) {
        std::cout << static_cast<uint32_t>(item) << std::endl;
    }

}
