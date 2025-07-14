//
// Created by liaohy on 7/14/25.
//

#pragma once

#include <cstdint>
#include <atomic>
#include <string.h>

template<uint32_t N>
class RingBuffer {
public:
    RingBuffer() {
        read_handle.store(&buffer[0]) ;
        write_handle.store(read_handle.load());
    };

    static consteval uint32_t overall_len() { return up_power_of_2(N)-1; }
    uint32_t get_len() { return (up_power_of_2(N)+write_handle.load()-read_handle.load()) & overall_len();}
    uint32_t get_read_index() { return  read_handle.load() - &buffer[0];}
    uint32_t get_write_index() { return write_handle.load() - &buffer[0];}

    const uint8_t operator[](const uint32_t index) { return *(buffer+((get_read_index()+index)&overall_len())); }
    uint8_t* get() {return read_handle.load(); }

    bool write_data(uint8_t* data, uint32_t len) {
        if (overall_len()-get_len() < len)
            return false;
        if (overall_len() - get_write_index() >= len) {
            memcpy(write_handle.load(), data, len);
            add_write(len);
            return true;
        }
        memcpy(write_handle.load(), data, up_power_of_2(N) - get_write_index());
        // add_write(up_power_of_2(N) - get_write_index());
        memcpy(buffer, data + up_power_of_2(N) - get_write_index(), len-(up_power_of_2(N)-get_write_index()));
        add_write(len);
        return true;
    }

    bool get_data(uint8_t*data, uint32_t len) {
        if (get_len() < len)
            return false;
        if (read_handle.load() < write_handle.load()) {
            memcpy(data, read_handle.load(), len);
            add_read(len);
            return true;
        }
        memcpy(data, read_handle.load(), up_power_of_2(N) - get_read_index());
        memcpy(data + up_power_of_2(N) - get_read_index(), buffer, len-(up_power_of_2(N)-get_read_index()));
        add_read(len);
        return true;
    }

    void add_write(uint32_t len) {write_handle.store(&buffer[0]+((get_write_index()+len)&overall_len()));}
    void add_read(uint32_t len = 1) {read_handle.store(&buffer[0]+((get_read_index()+len)&overall_len()));}

private:
    static consteval uint32_t up_power_of_2(uint32_t number) {
        if (number == 0)
            return 2;
        while (number & number - 1)
            ++number;
        return number;
    }

    uint8_t buffer[up_power_of_2(N)];

    std::atomic<uint8_t*> write_handle;
    std::atomic<uint8_t*> read_handle;


};
