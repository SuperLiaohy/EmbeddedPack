//
// Created by liaohy on 8/31/25.
//

#pragma once

#include <cstdint>
#include <atomic>
#include <cstring>
#include <vector>
#include <concepts>

namespace EP::Component {
template<typename T>
concept isContainer = requires(T t) {
    {t.size()} -> std::convertible_to<uint32_t>;
};

namespace RingBufferHandle {
    class Normal {
    public:
        Normal() : write_handle(0), read_handle(0) {};
        uint32_t write_handle;
        uint32_t read_handle;
    };
    class Atomic {
    public:
        Atomic() : write_handle(0), read_handle(0) {};
        std::atomic<uint32_t> write_handle;
        std::atomic<uint32_t> read_handle;
    };

    template<typename T>
    concept isHandle = requires(T t) {
        t.write_handle;
        t.read_handle;
    };
}

template<uint32_t N, typename T = uint8_t, RingBufferHandle::isHandle Handle = RingBufferHandle::Normal>
class RingBufferStatic : public Handle {
public:
    explicit RingBufferStatic() : Handle() {};

    const T* get_container() const {return buffer;}

    [[nodiscard]] bool is_empty() const { return Handle::write_handle == Handle::read_handle; }
    [[nodiscard]] bool is_full() const { return ((Handle::write_handle + 1) & mask()) == Handle::read_handle; }

    // 返回缓冲区掩码（总是 2 的幂次方减 1）
    [[nodiscard]] consteval uint32_t mask() { return capacity() - 1; }

    // 返回缓冲区实际容量（向上取整为 2 的幂次方）
    [[nodiscard]] consteval uint32_t capacity() { return up_power_of_2(N); }

    // 返回当前缓冲区中的元素数量
    [[nodiscard]] uint32_t size() const {
        return (capacity() + Handle::write_handle - Handle::read_handle) & mask();
    }

    // 返回当前可用空间数量
    [[nodiscard]] uint32_t available() const {
        return mask() - size();
    }

    [[nodiscard]] uint32_t get_read_index() const { return Handle::read_handle; }
    [[nodiscard]] uint32_t get_write_index() const { return Handle::write_handle; }

    // 安全地访问元素，确保索引在范围内
    T operator[](const uint32_t index) const {
        return buffer[(Handle::read_handle + index) & mask()];
    }

    T head() const { return buffer[Handle::read_handle]; }

    bool write_data(const T* data, uint32_t len) {
        if (available() < len)
            return false;

        const uint32_t first_chunk = std::min(capacity() - (Handle::write_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(&buffer[Handle::write_handle & mask()], data, first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），复制到缓冲区开头
        if (second_chunk > 0) {
            std::memcpy(&buffer[0], data + first_chunk, second_chunk * sizeof(T));
        }

        // 更新写指针
        Handle::write_handle = (Handle::write_handle + len) & mask();
        return true;
    }

    bool write_data_force(const T* data, uint32_t len) {
        if (available() < len)
            Handle::read_handle = (Handle::read_handle + len - available()) & mask();

        const uint32_t first_chunk = std::min(capacity() - (Handle::write_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(&buffer[Handle::write_handle & mask()], data, first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），复制到缓冲区开头
        if (second_chunk > 0) {
            std::memcpy(&buffer[0], data + first_chunk, second_chunk * sizeof(T));
        }

        // 更新写指针
        Handle::write_handle = (Handle::write_handle + len) & mask();
        return true;
    }

    bool get_data(T* data, uint32_t len) {
        if (size() < len)
            return false;

        const uint32_t first_chunk = std::min(capacity() - (Handle::read_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(data, &buffer[Handle::read_handle & mask()], first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），从缓冲区开头复制
        if (second_chunk > 0) {
            std::memcpy(data + first_chunk, &buffer[0], second_chunk * sizeof(T));
        }

        // 更新读指针
        Handle::read_handle = (Handle::read_handle + len) & mask();
        return true;
    }

    bool push(const T& value) {
        if (is_full()) {return false;}
        buffer[Handle::write_handle] = value;
        add_write();
        return true;
    }
    void push_force(const T& value) {
        if (is_full()) {add_read();}
        buffer[Handle::write_handle] = value;
        add_write();
    }
    T pop() {
        if (is_empty()) {return T();}
        const T& temp = buffer[Handle::read_handle];
        add_read();
        return temp;
    }
    void pop(T& value) {
        if (is_empty()) {return;}
        value = buffer[Handle::read_handle];
        add_read();
    }

    void add_write(uint32_t len = 1) { Handle::write_handle = (Handle::write_handle + len) & mask(); }
    void add_read(uint32_t len = 1) { Handle::read_handle = (Handle::read_handle + len) & mask(); }

    void reset() {
        Handle::read_handle = 0;
        Handle::write_handle = 0;
    }

private:
    consteval static uint32_t up_power_of_2(uint32_t number) {
        if (number == 0)
            return 2;
        number--;
        for (uint32_t i = 1; i < sizeof(uint32_t) * 8; i <<= 1)
            number |= number >> i;
        return number + 1;
    }

    T buffer[up_power_of_2(N)];
};

template<typename T = uint8_t, isContainer Container = std::vector<T>, RingBufferHandle::isHandle Handle = RingBufferHandle::Normal>
class RingBuffer : public Handle {
public:
    explicit RingBuffer(const uint32_t N) : Handle(), buffer(up_power_of_2(N)) {};

    const Container& get_container() const {return buffer;}

    Container get_valid_container() const {
        uint32_t len = size();
        Container ret(len);
        const uint32_t first_chunk = std::min(capacity() - (Handle::read_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(static_cast<T *>(ret.data()), &buffer[Handle::read_handle & mask()], first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），从缓冲区开头复制
        if (second_chunk > 0) {
            std::memcpy(static_cast<T *>(ret.data()) + first_chunk, &buffer[0], second_chunk * sizeof(T));
        }

        return ret;
    }

    [[nodiscard]] bool is_empty() const { return Handle::write_handle == Handle::read_handle; }
    [[nodiscard]] bool is_full() const { return ((Handle::write_handle + 1) & mask()) == Handle::read_handle; }

    // 返回缓冲区掩码（总是 2 的幂次方减 1）
    [[nodiscard]] uint32_t mask() const { return capacity() - 1; }

    // 返回缓冲区实际容量（向上取整为 2 的幂次方）
    [[nodiscard]] uint32_t capacity() const { return buffer.size(); }

    // 返回当前缓冲区中的元素数量
    [[nodiscard]] uint32_t size() const {
        return (capacity() + Handle::write_handle - Handle::read_handle) & mask();
    }

    // 返回当前可用空间数量
    [[nodiscard]] uint32_t available() const {
        return mask() - size();
    }

    [[nodiscard]] uint32_t get_read_index() const { return Handle::read_handle; }
    [[nodiscard]] uint32_t get_write_index() const { return Handle::write_handle; }

    // 安全地访问元素，确保索引在范围内
    T operator[](const uint32_t index) const {
        if (index >= size()) {
            // 可以选择抛出异常或返回默认值
            return T{};
        }
        return buffer[(Handle::read_handle + index) & mask()];
    }

    T head() const { return buffer[Handle::read_handle]; }

    bool write_data(const T* data, uint32_t len) {
        if (available() < len)
            return false;

        const uint32_t first_chunk = std::min(capacity() - (Handle::write_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(&buffer[Handle::write_handle & mask()], data, first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），复制到缓冲区开头
        if (second_chunk > 0) {
            std::memcpy(&buffer[0], data + first_chunk, second_chunk * sizeof(T));
        }

        // 更新写指针
        Handle::write_handle = (Handle::write_handle + len) & mask();
        return true;
    }

    bool write_data_force(const T* data, uint32_t len) {
        if (available() < len)
            Handle::read_handle = (Handle::read_handle + len - available()) & mask();

        const uint32_t first_chunk = std::min(capacity() - (Handle::write_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(&buffer[Handle::write_handle & mask()], data, first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），复制到缓冲区开头
        if (second_chunk > 0) {
            std::memcpy(&buffer[0], data + first_chunk, second_chunk * sizeof(T));
        }

        // 更新写指针
        Handle::write_handle = (Handle::write_handle + len) & mask();
        return true;
    }

    bool get_data(T* data, uint32_t len) {
        if (size() < len)
            return false;

        const uint32_t first_chunk = std::min(capacity() - (Handle::read_handle & mask()), len);
        const uint32_t second_chunk = len - first_chunk;

        // 复制第一段数据（可能是全部）
        std::memcpy(data, &buffer[Handle::read_handle & mask()], first_chunk * sizeof(T));

        // 如果有第二段数据（环绕情况），从缓冲区开头复制
        if (second_chunk > 0) {
            std::memcpy(data + first_chunk, &buffer[0], second_chunk * sizeof(T));
        }

        // 更新读指针
        Handle::read_handle = (Handle::read_handle + len) & mask();
        return true;
    }

    bool push(const T& value) {
        if (is_full()) {return false;}
        buffer[Handle::write_handle] = value;
        add_write();
        return true;
    }
    void push_force(const T& value) {
        if (is_full()) {add_read();}
        buffer[Handle::write_handle] = value;
        add_write();
    }
    T pop() {
        if (is_empty()) {return T();}
        const T& temp = buffer[Handle::read_handle];
        add_read();
        return temp;
    }
    void pop(T& value) {
        if (is_empty()) {return;}
        value = buffer[Handle::read_handle];
        add_read();
    }

    void add_write(uint32_t len = 1) { Handle::write_handle = (Handle::write_handle + len) & mask(); }
    void add_read(uint32_t len = 1) { Handle::read_handle = (Handle::read_handle + len) & mask(); }

    void reset() {
        Handle::read_handle = 0;
        Handle::write_handle = 0;
    }

private:
    uint32_t up_power_of_2(uint32_t number) {
        if (number == 0)
            return 2;
        number--;
        for (uint32_t i = 1; i < sizeof(uint32_t) * 8; i <<= 1)
            number |= number >> i;
        return number + 1;
    }

    Container buffer;
};
}
