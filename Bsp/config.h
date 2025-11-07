//
// Created by liaohy on 7/1/25.
//

#pragma once
#include <type_traits>
#include <concepts>
#include <cstdint>
template<typename T>
concept input_pl = requires(T t) {
    t.input_init();
    typename T::state;
    {t.read()} ->  std::same_as<typename T::state>;
};

template<typename T>
concept output_pl = requires(T t)
{
    t.output_init();
    t.write_up();
    t.write_down();
    t.toggle();
    typename T::state;
    t.write(std::declval<typename T::state>());
};

template<typename T>
concept gpio_pl = input_pl<T> and output_pl<T>;


template<typename T>
concept i2c_pl = requires(T t) {
    t.init();

    {t.receive_byte()} -> std::convertible_to<uint8_t>;
    t.transmit_byte();

    t.receive_buffer(std::declval<uint8_t*>(),std::declval<uint8_t>());
    t.transmit_buffer(std::declval<uint8_t*>(),std::declval<uint8_t>());
};

