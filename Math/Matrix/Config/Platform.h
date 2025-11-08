//
// Created by liaohy on 3/17/25.
//
#pragma once

#include "concepts"
#include "cstdint"

template<typename T>
concept mathPl = requires(T t) {
    typename T::status;
    typename T::MatrixInstance;
    std::declval<typename T::MatrixInstance>().pData;

    T::matrix_init(std::declval<typename T::MatrixInstance *>(),
                   uint16_t(), uint16_t(), std::declval<float *>());

    T::matrix_add(std::declval<typename T::MatrixInstance *>(),
                  std::declval<typename T::MatrixInstance *>(),
                  std::declval<typename T::MatrixInstance *>());

    T::matrix_sub(std::declval<typename T::MatrixInstance *>(),
                  std::declval<typename T::MatrixInstance *>(),
                  std::declval<typename T::MatrixInstance *>());

    T::matrix_mul(std::declval<typename T::MatrixInstance *>(),
                  std::declval<typename T::MatrixInstance *>(),
                  std::declval<typename T::MatrixInstance *>());

    T::matrix_scale(std::declval<typename T::MatrixInstance *>(),
                    std::declval<float>(),
                    std::declval<typename T::MatrixInstance *>());

    T::matrix_trans(std::declval<typename T::MatrixInstance *>(),
                    std::declval<typename T::MatrixInstance *>());

    T::matrix_inverse(std::declval<typename T::MatrixInstance *>(),
                    std::declval<typename T::MatrixInstance *>());
};


