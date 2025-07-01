//
// Created by liaohy on 7/1/25.
//

#pragma once

// I2C协议，SCL和SDA起始都是高电平。
// 每八位一个Ack
// SDA需要在SCL的高电平稳定数据，在SCL的低电平改变数据
// I2C是线与的关系
#include <./concept.h>
#include <cstdint>

template<gpio_pl io>
class SoftI2C {
    enum class state {
        ACK,
        NACK
    };
public:
    SoftI2C(const io& scl, const io& sda) : scl(scl), sda(sda) {};

    void init();

    template<auto delay>
    state transmit_device_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t byte);

    template<auto delay>
    void receive_device_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t* byte);

    template<auto delay>
    state transmit_device_buffer(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buffer, uint8_t len);

    template<auto delay>
    void receive_device_buffer(uint8_t dev_addr, uint8_t reg_addr, uint8_t* buffer, uint8_t len);

    template<auto delay>
    state transmit_byte(uint8_t byte);

    template<auto delay>
    uint8_t receive_byte();

    template<auto delay>
    state transmit_buffer(uint8_t *buffer, uint8_t len);

    template<auto delay>
    void receive_buffer(uint8_t* buffer, uint8_t len);

protected:
    template<auto delay>
    state send_byte(uint8_t data);

    template<auto delay>
    uint8_t read_byte(state s);

    template<auto delay>
    state send_bytes(uint8_t* bytes, uint8_t len);

    template<auto delay>
    void read_bytes(uint8_t* bytes, uint8_t len);

    template<auto delay>
    void begin();

    template<auto delay>
    state read_ack();

    template<auto delay>
    void ack();

    template<auto delay>
    void nack();

    template<auto delay>
    void stop();

    io scl;
    io sda;
};


template<gpio_pl io>
void SoftI2C<io>::init() {
    scl.output_init();
    sda.output_init();
    scl.write_up();
    sda.write_up();
}

template<gpio_pl io>
template<auto delay>
typename SoftI2C<io>::state SoftI2C<io>::transmit_device_buffer(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buffer, uint8_t len) {
    this->begin<delay>();
    if (this->send_byte<delay>(dev_addr))
        return state::NACK;
    if (this->send_byte<delay>(reg_addr))
        return state::NACK;
    if (this->send_bytes<delay>(buffer, len))
        return state::NACK;
    this->stop<delay>();
    return state::ACK;
}

template<gpio_pl io>
template<auto delay>
void SoftI2C<io>::receive_device_buffer(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buffer, uint8_t len) {
    this->begin<delay>();
    if (this->send_byte<delay>(dev_addr))
        return;
    if (this->send_byte<delay>(reg_addr))
        return;
    this->begin<delay>();
    this->read_bytes<delay>(buffer, len);
    this->stop<delay>();
}

template<gpio_pl io>
template<auto delay>
typename SoftI2C<io>::state SoftI2C<io>::transmit_device_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t byte) {
    this->begin<delay>();
    if (this->send_byte<delay>(dev_addr))
        return state::NACK;
    if (this->send_byte<delay>(reg_addr))
        return state::NACK;
    if (this->send_byte<delay>(byte))
        return state::NACK;
    this->stop<delay>();
    return state::ACK;
}

template<gpio_pl io>
template<auto delay>
void SoftI2C<io>::receive_device_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t* byte) {
    this->begin<delay>();
    if (this->send_byte<delay>(dev_addr))
        return ;
    if (this->send_byte<delay>(reg_addr))
        return ;
    this->begin<delay>();
    *byte = this->read_byte<delay>(state::NACK);
    this->stop<delay>();
}

template<gpio_pl io>
template<auto delay>
typename SoftI2C<io>::state SoftI2C<io>::transmit_buffer(uint8_t *buffer, uint8_t len) {
    this->begin<delay>();
    if (this->send_bytes<delay>(buffer, len))
        return state::NACK;
    this->stop<delay>();
    return state::ACK;
}

template<gpio_pl io>
template<auto delay>
void SoftI2C<io>::receive_buffer(uint8_t *buffer, uint8_t len) {
    this->begin<delay>();
    this->read_bytes<delay>(buffer, len);
    this->stop<delay>();
}

template<gpio_pl io>
template<auto delay>
typename SoftI2C<io>::state SoftI2C<io>::transmit_byte(uint8_t byte) {
    this->begin<delay>();
    if (this->send_byte<delay>(byte))
        return state::NACK;
    this->stop<delay>();
    return state::ACK;
}

template<gpio_pl io>
template<auto delay>
uint8_t SoftI2C<io>::receive_byte() {
    this->begin<delay>();
    uint8_t byte = this->read_byte<delay>(state::NACK);
    this->stop<delay>();
    return byte;
}

template<gpio_pl io>
template<auto delay>
typename SoftI2C<io>::state SoftI2C<io>::send_bytes(uint8_t* bytes, uint8_t len) {
    for (int i = 0; i < len; ++i) {
        if(send_bytes<delay>(bytes[i]) == state::NACK)
            return state::NACK;
    }
    return state::ACK;
}

template<gpio_pl io>
template<auto delay>
void SoftI2C<io>::read_bytes(uint8_t *bytes, uint8_t len) {
    for (int i = 0; i < len - 1; ++i) {
        bytes[i] = send_bytes<delay>(state::ACK);
    }
    send_bytes<delay>(state::NACK);
}



/**
 *
 * @tparam delay 用户可以传入自定义的延时函数
 * @brief I2C的起始信号,函数内部中已保证，起始时sda和scl为高电平，且sda为output，结束时确保里scl为低电平
 */
template<gpio_pl io>
template<auto delay>
void SoftI2C<io>::begin() {
    this->sda.output();
    this->sda.write_up();
    this->scl.write_up();
    delay(1);
    this->sda.write_down();
    delay(1);
    this->scl.write_down();
    // delay(1);
}

/**
 *
 * @tparam delay 用户可以传入自定义的延时函数
 * @param data 要发送的 uint8_t 字节
 * @return 返回为从机是否应答
 * @brief I2C的发送字节，函数内部中已保证，起始时sda为output，scl为低电平，结束时为scl低电平衔接read_ack
 */
template<gpio_pl io>
template<auto delay>
typename SoftI2C<io>::state SoftI2C<io>::send_byte(uint8_t data) {
    this->sda.output_init();
    for (int i = 0; i < 8; ++i) {
        this->scl.write_down();
        delay(1);
        if (data & 0x80)
            this->sda.write_up();
        else
            this->sda.write_down();
        data <<= 1;
        delay(1);
        this->scl.write_up();
        delay(1);
    }

    this->scl.write_down();
    delay(1);

    return read_ack<delay>();
}

/**
 *
 * @tparam delay 用户可以传入自定义延时函数
 * @param s 要发送ack还是nack
 * @return 返回读取到的 uint8_t data
 * @brief I2C的读取字节，函数内部中已保证，起始时sda为input，scl为低电平，结束时为scl低电平衔接ack or nack
 */
template<gpio_pl io>
template<auto delay>
uint8_t SoftI2C<io>::read_byte(state s) {
    this->sda.input_init();
    this->scl.write_down();
    delay(1);
    uint8_t data = 0;
    for (int i = 0; i < 8; ++i) {
        this->scl.write_up();
        delay(1);
        if (static_cast<bool>(this->sda.read()))
            data |= 1;
        data <<= 1;
        this->scl.write_down();
        delay(1);
    }
    if (s == state::ACK)
        this->ack<delay>();
    this->nack<delay>();
    return data;
}

/**
 *
 * @tparam delay 用户可以传入自定义延时函数
 * @return 返回为从机是否应答
 * @brief I2C的读取应答位，函数内部中已保证，起始时sda释放，sda为input，scl为高电平，结束时scl低电平。
 */
template<gpio_pl io>
template<auto delay>
typename SoftI2C<io>::state SoftI2C<io>::read_ack() {
    // sda.write_up();
    sda.input_init();
    // delay(1);
    scl.write_up();
    delay(1);
    uint8_t timeout = 0;
    state ack = state::ACK;
    while (sda.read()) {
        ++timeout;
        if (timeout > 100) {
            ack = state::NACK;
            break;
        }
    }
    scl.write_down();
    delay(1);
    return ack;

}

/**
 *
 * @tparam delay 用户可以传入自定义延时函数
 * @brief I2C的发送ack信号，函数内部已保证，起始时sda为output，scl为低电平，结束时scl为低电平
 */
template<gpio_pl io>
template<auto delay>
void SoftI2C<io>::ack() {
    // scl.write_down();
    sda.output_init();
    sda.write_down();
    // delay(1);
    scl.write_up();
    delay(1);
    scl.write_down();
    // delay(1);
}


/**
 *
 * @tparam delay 用户可以传入自定义延时函数
 * @brief I2C的发送nack信号，函数内部已保证，起始时sda为output，scl为低电平，结束时scl为低电平
 */
template<gpio_pl io>
template<auto delay>
void SoftI2C<io>::nack() {
    // scl.write_down();
    // sda.output_init();
    // sda.write_up();
    // delay(1);
    scl.write_up();
    delay(1);
    scl.write_down();
    // delay(1);
}

/**
 *
 * @tparam delay 用户可以传入自定义的延时函数
 * @brief I2C的终止信号,函数内部中已保证，起始时sda和scl为低电平，且sda为output，结束时sda和scl为高电平
 */
template<gpio_pl io>
template<auto delay>
void SoftI2C<io>::stop() {
    sda.output_init();
    scl.write_down();
    sda.write_down();
    delay(1);
    scl.write_up();
    // delay(1);
    sda.write_up();
    // delay(1);
}



