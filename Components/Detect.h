//
// Created by liaohy on 11/7/25.
//

#pragma once

// #include "config.h"
#include <cstdint>

namespace EP::Component {
namespace detectDep {
    using fun = void(void *);
    enum State : uint8_t{
        WORKING,
        MISSING,
        RECOVER,
        LOSE
    };
}
template<auto sysTime>
class Detect {
    using fun = detectDep::fun;
    using State = detectDep::State;
public:
    explicit Detect(const uint32_t maxInterval, fun*workingFun=nullptr, fun*missingFun=nullptr, fun*recoverFun=nullptr, fun*loseFun=nullptr) : maxInterval(maxInterval), workingFun(workingFun), missingFun(missingFun), recoverFun(recoverFun), loseFun(loseFun) {}
    template<auto getSystime = sysTime>
    void update() {lastUpdate = getSystime();}
    template<auto getSystime = sysTime>
    [[nodiscard]] State detect() {
        if (getSystime() - lastUpdate >= maxInterval)
            if (s == State::LOSE || s == State::MISSING) s = State::MISSING; else s = State::LOSE;
        else
            if (s == State::RECOVER || s == State::WORKING) s = State::WORKING; else s = State::RECOVER;
        return s;
    }
    void working(void *parament = nullptr) const { if (workingFun) workingFun(parament); }
    void missing(void *parament = nullptr) const { if (missingFun) missingFun(parament); }
    void recover(void *parament = nullptr) const { if (recoverFun) recoverFun(parament); }
    void lose(void *parament = nullptr) const { if (loseFun) loseFun(parament); }

    ~Detect() {std::cout << "Detect::~Detect()" <<std::endl;}
private:
    uint32_t maxInterval = 0;
    uint32_t lastUpdate = 0;
    State s = State::WORKING;
    fun *workingFun;
    fun *missingFun;
    fun *recoverFun;
    fun *loseFun;
};
}
