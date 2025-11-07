//
// Created by liaohy on 25-3-15.
//
#pragma once
#include "Matrix.hpp"
template<uint32_t COLS>
using RowVec = Matrix<1, COLS>;


template<>
inline Matrix<1, 3> Matrix<1, 3>::operator^(const Matrix<1, 3> &other) const {
    Matrix result;
    result(1) = (*this)(2) * other(3) - (*this)(3) * other(2);
    result(2) = (*this)(3) * other(1) - (*this)(1) * other(3);
    result(3) = (*this)(1) * other(2) - (*this)(2) * other(1);
    return result;
}