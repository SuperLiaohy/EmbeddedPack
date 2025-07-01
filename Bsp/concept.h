//
// Created by liaohy on 7/1/25.
//

#pragma once
#include <type_traits>
#include <cstdint>
template<typename T>
concept input_pl = requires(T t) {
    t.init();
    typename T::state;
    {t.read()} ->  typename T::state;
};

template<typename T>
concept output_pl = requires(T t)
{
    t.init();
    t.write_up();
    t.write_down();
    t.toggle();
    typename T::state;
    t.write(decltype(typename T::state){});
};

template<typename T>
concept gpio_pl = requires(T t) {
    t.input_init();
    typename T::state;
    {t.read()} ->  typename T::state;

    t.output_init();
    t.write_up();
    t.write_down();
    t.toggle();
    t.write(decltype(typename T::state){});
};


template<typename T>
concept i2c_pl = requires(T t) {
    t.init();

    {t.receive_byte()} -> std::remove_reference_t<uint8_t>;
    t.transmit_byte();

    t.receive_buffer(std::declval<uint8_t*>{},std::declval<uint8_t>{});
    t.transmit_buffer(std::declval<uint8_t*>{},std::declval<uint8_t>{});
};

