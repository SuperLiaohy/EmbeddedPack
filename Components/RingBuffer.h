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
    RingBuffer() {
        read_handle.store(&buffer[0]) ;
        write_handle.store(read_handle.load());
    };

    static consteval uint32_t overall_len() { return up_power_of_2(N)-1; }
    uint32_t get_len() { return (up_power_of_2(N)+write_handle.load()-read_handle.load()) & overall_len();}
    uint32_t get_read_index() { return  read_handle.load() - &buffer[0];}
    uint32_t get_write_index() { return write_handle.load() - &buffer[0];}

    const T operator[](const uint32_t index) { return *(buffer+((get_read_index()+index)&overall_len())); }
    T* get() {return read_handle.load(); }

    bool write_data(T* data, uint32_t len) {
        if (overall_len()-get_len() < len)
            return false;
        if (overall_len() - get_write_index() >= len) {
            memcpy(write_handle.load(), data, len*sizeof(T));
            add_write(len);
            return true;
        }
        memcpy(write_handle.load(), data, (up_power_of_2(N) - get_write_index())*sizeof(T));
        // add_write(up_power_of_2(N) - get_write_index());
        memcpy(buffer, data + up_power_of_2(N) - get_write_index(), (len-(up_power_of_2(N)-get_write_index()))*sizeof(T));
        add_write(len);
        return true;
    }

    bool get_data(T*data, uint32_t len) {
        if (get_len() < len)
            return false;
        if (read_handle.load() < write_handle.load()) {
            memcpy(data, read_handle.load(), len*sizeof(T));
            add_read(len);
            return true;
        }
        memcpy(data, read_handle.load(), (up_power_of_2(N) - get_read_index())*sizeof(T));
        memcpy(data + up_power_of_2(N) - get_read_index(), buffer, (len-(up_power_of_2(N)-get_read_index()))*sizeof(T));
        add_read(len);
        return true;
    }

    void add_write(uint32_t len) {write_handle.store(&buffer[0]+((get_write_index()+len)&overall_len()));}
    void add_read(uint32_t len = 1) {read_handle.store(&buffer[0]+((get_read_index()+len)&overall_len()));}
    void reset() {
        read_handle.store(&buffer[0]) ;
        write_handle.store(read_handle.load());
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

    std::atomic<T*> write_handle;
    std::atomic<T*> read_handle;


};
