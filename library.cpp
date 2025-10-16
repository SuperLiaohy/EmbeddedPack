#include <array>

#include "library.h"
#include "Components/RingBuffer.h"
#include <iostream>

int main() {
    RingBuffer buffer(6);
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
