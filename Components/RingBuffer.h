//
// Created by liaohy on 7/14/25.
//

#pragma once

#include <cstdint>
#include <atomic>
#include <string.h>

template<uint32_t N, typename T = uint8_t>
class RingBuffer {
public:
    RingBuffer() : read_handle(0), write_handle(0) {};

    static consteval uint32_t overall_index() { return up_power_of_2(N)-1; }
    uint32_t get_index_len() { return (up_power_of_2(N)+write_handle.load()-read_handle.load()) & overall_index();}
    uint32_t get_read_index() { return  read_handle.load();}
    uint32_t get_write_index() { return write_handle.load();}

    const T operator[](const uint32_t index) { return *(buffer+((get_read_index()+index)&overall_index())); }
    T get() {return buffer[read_handle.load()]; }

    bool write_data(const T* data, uint32_t len) {
        if (overall_index()-get_index_len() < len)
            return false;
        if (overall_index() - get_write_index() >= len) {
            memcpy(&buffer[write_handle.load()], data, len*sizeof(T));
            add_write(len);
            return true;
        }
        memcpy(&buffer[write_handle.load()], data, (up_power_of_2(N) - get_write_index())*sizeof(T));
        memcpy(buffer, data + up_power_of_2(N) - get_write_index(), (len-(up_power_of_2(N)-get_write_index()))*sizeof(T));
        add_write(len);
        return true;
    }

    bool get_data(T*data, uint32_t len) {
        if (get_index_len() < len)
            return false;
        if (read_handle.load() < write_handle.load()) {
            memcpy(data, &buffer[read_handle.load()], len*sizeof(T));
            add_read(len);
            return true;
        }
        memcpy(data, &buffer[read_handle.load()], (up_power_of_2(N) - get_read_index())*sizeof(T));
        memcpy(data + up_power_of_2(N) - get_read_index(), buffer, (len-(up_power_of_2(N)-get_read_index()))*sizeof(T));
        add_read(len);
        return true;
    }

    void add_write(uint32_t len) {write_handle.store((get_write_index()+len)&overall_index());}
    void add_read(uint32_t len = 1) {read_handle.store((get_read_index()+len)&overall_index());}
    void reset() {
        read_handle.store(0) ;
        write_handle.store(0);
    }
private:
    static consteval uint32_t up_power_of_2(uint32_t number) {
        if (number == 0)
            return 2;
        while (number & number - 1)
            ++number;
        return number;
    }

    T buffer[up_power_of_2(N)];

    std::atomic<uint32_t> write_handle;
    std::atomic<uint32_t> read_handle;


};