//
// Created by liaohy on 11/7/25.
//

#pragma once

// #include "config.h"
#include <cstdint>

namespace EP::Component {
class Detect {
public:
    using fun = void(void *);
    enum State : uint8_t{
        WORKING,
        MISSING,
        RECOVER,
        LOSE
    };
    explicit Detect(const uint32_t maxInterval, fun*workingFun=nullptr, fun*missingFun=nullptr, fun*recoverFun=nullptr, fun*loseFun=nullptr) : maxInterval(maxInterval), workingFun(workingFun), missingFun(missingFun), recoverFun(recoverFun), loseFun(loseFun) {}
    template<auto getSystime>
    void update() {lastUpdate = getSystime();}
    template<auto getSystime>
    [[nodiscard]] State detect() {
        if (getSystime() - lastUpdate >= maxInterval)
            if (s == LOSE || s == MISSING) s = MISSING; else s = LOSE;
        else
            if (s == RECOVER || s == WORKING) s = WORKING; else s = RECOVER;
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
    State s = WORKING;
    fun *workingFun;
    fun *missingFun;
    fun *recoverFun;
    fun *loseFun;
};
}
