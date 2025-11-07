//
// Created by liaohy on 11/7/25.
//

#pragma once

#include <cstdint>
namespace EP::Component {
class Count {
public:
    Count() = default;

    [[nodiscard]] uint32_t getCount() const {return count;}
    [[nodiscard]] uint32_t getMax() const {return max;}

    void operator++() {++count;}

    void record() {
        max = count;
        count = 0;
    }
private:
    uint32_t count = 0;
    uint32_t max = 0;
};
}
