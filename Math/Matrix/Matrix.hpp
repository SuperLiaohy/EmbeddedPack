//
// Created by Administrator on 25-1-4.
//

/*
 * 经过测试直接调用arm_math库和数组，用时为A
 * 通过Matrix类封装arm_math库和数组，用时为B=1.18A左右
 * 通过Matrix类封装直接用数组进行矩阵运算，用时为C=1.2B~1.3B左右
 */
#pragma once


#include <concepts>
#include "Config/Config.h"
#include "cstring"
#include "array"

template<uint32_t ROWS, uint32_t COLS, PlatFormConcept pl = Windows>
class Matrix : public pl {
public:
    consteval static bool is_point() { return (ROWS == 1 && COLS == 1); }

    consteval static bool is_row() { return (COLS != 1 && ROWS == 1); }

    consteval static bool is_col() { return (COLS == 1 && ROWS != 1); }

    consteval static bool is_vec() { return ((COLS == 1 && ROWS != 1) || (COLS != 1 && ROWS == 1)); }

    consteval static bool is_square() { return COLS == ROWS; }

    template<uint32_t R, uint32_t C, PlatFormConcept P>
    friend
    class Matrix;

public:
    constexpr static Matrix zeros() {
        Matrix result;
        memset(result.data, 0, ROWS * COLS * sizeof(float));
        return result;
    }

    constexpr static Matrix eyes() {
        Matrix result = zeros();

        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (i == j) {
                    result.data[i][j] = 1;
                }
            }
        }
        return result;
    }
// 常规矩阵
public:
    constexpr Matrix() {
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
    }

    constexpr Matrix(const Matrix &other) {
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, other.data, ROWS * COLS * sizeof(float));
    }

    constexpr Matrix &operator=(const Matrix &other) {
        if (&other == this) { return *this; }
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, other.data, ROWS * COLS * sizeof(float));
        return *this;
    }

    constexpr Matrix(Matrix &&other) noexcept {
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, other.data, ROWS * COLS * sizeof(float));
    }

    constexpr Matrix &operator=(Matrix &&other) noexcept {
        if (&other == this) { return *this; }
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, other.data, ROWS * COLS * sizeof(float));
        return *this;
    }

    ~Matrix() = default;

    constexpr explicit Matrix(const float (&arr)[ROWS][COLS]) : Matrix() {
        memcpy(this->data, arr, ROWS * COLS * sizeof(float));
    }

    constexpr Matrix &operator=(const float (&arr)[ROWS][COLS]) {
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, arr, ROWS * COLS * sizeof(float));
        return *this;
    }

    // 重载赋值运算符处理nullptr
    constexpr Matrix &operator=(std::nullptr_t) {
        matrix.pData = nullptr;
    }

    constexpr bool operator==(std::nullptr_t) {
        return matrix.pData == nullptr;
    }

    constexpr uint32_t get_row() { return ROWS; }

    constexpr uint32_t get_col() { return COLS; }

    float &operator()(uint32_t row, uint32_t col) { return data[row - 1][col - 1]; }

    constexpr float operator()(uint32_t row, uint32_t col) const { return data[row - 1][col - 1]; }

    constexpr Matrix operator+(const Matrix &other) const {
        Matrix result;
        pl::matrix_add(&matrix, &other.matrix, &result.matrix);
        return result;
    }

    constexpr Matrix operator-(const Matrix &other) const {
        Matrix result;
        pl::matrix_sub(&matrix, &other.matrix, &result.matrix);
        return result;
    }

    template<uint32_t col>
    constexpr Matrix<ROWS, col> operator*(const Matrix<COLS, col> &other) const {
        Matrix<ROWS, col> result;
        pl::matrix_mul(&matrix, &other.matrix, &result.matrix);
        return result;
    }

    constexpr Matrix operator*(float scale) const {
        Matrix result;
        pl::matrix_scale(&matrix, scale, &result.matrix);
        return result;
    }

    constexpr friend Matrix<ROWS, COLS> operator*(float scale, const Matrix<ROWS, COLS> &other) {
        Matrix result;
        pl::matrix_scale(&other.matrix, scale, &result.matrix);
        return result;
    }

    constexpr Matrix operator/(float scale) const {
        Matrix result;
        pl::matrix_scale(&matrix, 1.0f / scale, &result.matrix);
        return result;
    }

    constexpr Matrix<COLS, ROWS> transpose() const {
        Matrix<COLS, ROWS> result;
        pl::matrix_trans(&matrix, &result.matrix);
        return result;
    }

    constexpr void transpose(Matrix &mat) const requires(is_square()) {
        pl::matrix_trans(&matrix, &mat.matrix);
    }

    constexpr Matrix<COLS, ROWS> operator~() {
        Matrix<COLS, ROWS> result;
        pl::matrix_trans(&matrix, &result.matrix);
        return result;
    }

    void operator+=(const Matrix &other) {
        pl::matrix_add(&matrix, &other.matrix, &matrix);
    }

    void operator-=(const Matrix &other) {
        pl::matrix_sub(&matrix, &other.matrix, &matrix);
    }

    void operator*=(const Matrix<COLS, COLS> &other) {
        pl::matrix_mul(&matrix, &other.matrix, &matrix);
    }

    void operator*=(float scale) {
        pl::matrix_scale(&matrix, scale, &matrix);
    }

    void operator/=(float scale) {
        pl::matrix_scale(&matrix, 1.0f / scale, &matrix);
    }

    inline pl::status inv(Matrix &result) requires(is_square());

    inline Matrix inv() requires(is_square());

    template<uint32_t row, uint32_t clo>
    inline void
    assign_multiply(const Matrix<ROWS, clo> &first_matrix, const Matrix<row, COLS> &second_matrix) requires(row == clo);

    inline void assign_add(const Matrix &first_matrix, const Matrix &second_matrix);

    inline void assign_sub(const Matrix &first_matrix, const Matrix &second_matrix);

    inline void assign_scale(float scale);

    inline pl::status assign_inv(Matrix &scr_matrix) requires(is_square());

// 向量
public:
    /**
     * @brief 向量差乘 vector cross
     * @param other 另外一个向量 the other vector
     * @return Matrix 相同维度的向量 vector of the same dimension
     */
    Matrix operator^(const Matrix &other) const requires(is_vec() || is_point()) ;

    explicit Matrix(float other) requires(is_point()) : Matrix() {data[0][0] = other;};

// 列向量
public:

    constexpr explicit Matrix(const float (&arr)[ROWS]) requires(is_col()): Matrix<ROWS, 1>() {
        memcpy(this->data, arr, ROWS * sizeof(float));
    }

    constexpr explicit Matrix(const std::array<float, ROWS> &arr) requires(is_col()): Matrix<ROWS, 1>() {
        memcpy(this->data, arr.data(), COLS * sizeof(float));
    }

    constexpr float operator*(const Matrix<ROWS, 1> &other) const requires(is_col()) {
        Matrix<1, 1> result;
        Matrix<1, ROWS> tmp = this->transpose();
        pl::matrix_mul(&tmp.matrix, &other.matrix, &result.matrix);
        return result(1, 1);
    }

    float &operator[](uint32_t num) requires(is_col()) { return this->data[num][0]; };

    constexpr float operator[](uint32_t num) const requires(is_col()) { return this->data[num][0]; };

    float &operator()(uint32_t num) requires(is_col()) { return this->data[num - 1][0]; };

    constexpr float operator()(uint32_t num) const requires(is_col()) { return this->data[num - 1][0]; };

// 行向量
public:
    constexpr explicit Matrix(const float (&arr)[COLS]) requires(is_row()): Matrix<1, COLS>() {
        memcpy(this->data, arr, COLS * sizeof(float));
    }

    constexpr explicit Matrix(const std::array<float, COLS> &arr) requires(is_row()): Matrix<1, COLS>() {
        memcpy(this->data, arr.data(), COLS * sizeof(float));
    }

    constexpr float operator*(const Matrix<1, COLS> &other) const requires(is_row()) {
        Matrix<1, 1> result;
        Matrix<COLS, 1> tmp = other.transpose();
        pl::matrix_mul(&matrix, &tmp.matrix, &result.matrix);
        return result(1, 1);
    }

    float &operator[](uint32_t num) requires(is_row() || is_point()) { return this->data[0][num]; };

    constexpr float operator[](uint32_t num) const requires(is_row() || is_point()) { return this->data[0][num]; };

    float &operator()(uint32_t num) requires(is_row() || is_point()) { return this->data[0][num - 1]; };

    constexpr float operator()(uint32_t num) const requires(is_row() || is_point()) { return this->data[0][num - 1]; };


    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Matrix &mat) {
        os << "\n[";
        for (uint32_t i = 0; i < ROWS; ++i) {
            if (i > 0) os << "\n ";
            for (uint32_t j = 0; j < COLS; ++j) {
                os << " " << mat.data[i][j];
                if (j < COLS - 1) os << ",";
            }
            if (i < ROWS - 1) os << ",";
        }
        os << " ]";
        return os;
    }

private:
    pl::MatrixInstance matrix{};
    float data[ROWS][COLS]{};
};

template<uint32_t ROWS, uint32_t COLS, PlatFormConcept pl>
pl::status Matrix<ROWS, COLS, pl>::assign_inv(Matrix &scr_matrix) requires(is_square()) {
    typename pl::status ret = pl::matrix_inverse(&scr_matrix.matrix, &matrix);
    return ret;
}

template<uint32_t ROWS, uint32_t COLS, PlatFormConcept pl>
void Matrix<ROWS, COLS, pl>::assign_scale(float scale) {
    pl::matrix_scale(&matrix, scale, &matrix);
}

template<uint32_t ROWS, uint32_t COLS, PlatFormConcept pl>
void Matrix<ROWS, COLS, pl>::assign_sub(const Matrix &first_matrix, const Matrix &second_matrix) {
    pl::matrix_sub(&first_matrix.matrix, &second_matrix.matrix, &matrix);
}

template<uint32_t ROWS, uint32_t COLS, PlatFormConcept pl>
void Matrix<ROWS, COLS, pl>::assign_add(const Matrix &first_matrix, const Matrix &second_matrix) {
    pl::matrix_add(&first_matrix.matrix, &second_matrix.matrix, &matrix);
}

template<uint32_t ROWS, uint32_t COLS, PlatFormConcept pl>
template<uint32_t row, uint32_t clo>
void Matrix<ROWS, COLS, pl>::assign_multiply(const Matrix<ROWS, clo> &first_matrix,
                                             const Matrix<row, COLS> &second_matrix) requires(row == clo) {
    pl::matrxi_mul(&first_matrix.matrix, &second_matrix.matrix, &matrix);
}

template<uint32_t ROWS, uint32_t COLS, PlatFormConcept pl>
inline pl::status Matrix<ROWS, COLS, pl>::inv(Matrix &result) requires(is_square()) {
    arm_matrix_instance_f32 matrix_bak;
    matrix_bak.numRows = ROWS;
    matrix_bak.numCols = COLS;
    float data_bak[ROWS][COLS]{};
    memcpy(data_bak, data, ROWS * COLS * sizeof(float));
    matrix_bak.pData = reinterpret_cast<float *>(data_bak);
    typename pl::status ret = pl::matrix_inv(&matrix_bak, &result.matrix);
    return ret;
}

template<uint32_t ROWS, uint32_t COLS, PlatFormConcept pl>
inline Matrix<ROWS, COLS, pl> Matrix<ROWS, COLS, pl>::inv() requires(is_square()) {
    Matrix result;
    arm_matrix_instance_f32 matrix_bak;
    matrix_bak.numRows = ROWS;
    matrix_bak.numCols = COLS;
    float data_bak[ROWS][COLS]{};
    memcpy(data_bak, data, ROWS * COLS * sizeof(float));
    matrix_bak.pData = reinterpret_cast<float *>(data_bak);
    typename pl::status ret = pl::matrix_inverse(&matrix_bak, &result.matrix);
    return ret == ARM_MATH_SUCCESS ? result : zeros();
}
