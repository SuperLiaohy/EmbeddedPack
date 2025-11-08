//
// Created by Administrator on 25-1-4.
//

/*
 * 经过测试直接调用arm_math库和数组，用时为A
 * 通过Matrix类封装arm_math库和数组，用时为B=1.18A左右
 * 通过Matrix类封装直接用数组进行矩阵运算，用时为C=1.2B~1.3B左右
 */
#pragma once

#include <array>
#include "Config/Config.h"

template<uint32_t ROWS, uint32_t COLS, mathPl pl = Windows>
class Matrix : private pl {
public:
    consteval static bool is_point() { return (ROWS == 1 && COLS == 1); }

    consteval static bool is_row() { return (COLS != 1 && ROWS == 1); }

    consteval static bool is_col() { return (COLS == 1 && ROWS != 1); }

    consteval static bool is_vec() { return ((COLS == 1 && ROWS != 1) || (COLS != 1 && ROWS == 1)); }

    consteval static bool is_square() { return COLS == ROWS; }

    template<uint32_t R, uint32_t C, mathPl P>
    friend class Matrix;
public:
    constexpr static Matrix zeros() noexcept {
        Matrix result;
        memset(result.data, 0, ROWS * COLS * sizeof(float));
        return result;
    }

    constexpr static Matrix eyes() noexcept {
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
    constexpr Matrix() noexcept {
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
    }

    // copy constructor
    constexpr Matrix(const Matrix &other) noexcept : Matrix() {
        memcpy(data, other.data, ROWS * COLS * sizeof(float));
    }

    // copy assignment
    constexpr Matrix &operator=(const Matrix &other) noexcept {
        if (&other == this) { return *this; }
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, other.data, ROWS * COLS * sizeof(float));
        return *this;
    }

    // move constructor
    constexpr Matrix(Matrix &&other) noexcept {
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, other.data, ROWS * COLS * sizeof(float));
    }

    // move assignment
    constexpr Matrix &operator=(Matrix &&other) noexcept {
        if (&other == this) { return *this; }
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, other.data, ROWS * COLS * sizeof(float));
        return *this;
    }

    ~Matrix() = default;

    constexpr explicit Matrix(const float (&arr)[ROWS][COLS]) noexcept : Matrix() {
        memcpy(this->data, arr, ROWS * COLS * sizeof(float));
    }

    constexpr Matrix &operator=(const float (&arr)[ROWS][COLS]) noexcept {
        pl::matrix_init(&matrix, ROWS, COLS, reinterpret_cast<float32_t *>(data));
        memcpy(data, arr, ROWS * COLS * sizeof(float));
        return *this;
    }

    // 重载赋值运算符处理nullptr
    explicit Matrix(std::nullptr_t) noexcept {matrix.pData = nullptr;}
    Matrix& operator=(std::nullptr_t) noexcept {
        matrix.pData = nullptr;
        return *this;
    }

    constexpr bool operator==(std::nullptr_t) noexcept {return matrix.pData == nullptr;}
    constexpr bool operator!=(std::nullptr_t) noexcept {return matrix.pData != nullptr;}

    consteval uint32_t get_row() { return ROWS; }

    consteval uint32_t get_col() { return COLS; }

    float &operator()(const uint32_t row, const uint32_t col) noexcept { return data[row - 1][col - 1]; }

    constexpr float operator()(const uint32_t row, const uint32_t col) const noexcept { return data[row - 1][col - 1]; }

    constexpr Matrix operator+(const Matrix &other) const noexcept {
        Matrix result;
        pl::matrix_add(&matrix, &other.matrix, &result.matrix);
        return result;
    }

    constexpr Matrix operator-(const Matrix &other) const noexcept {
        Matrix result;
        pl::matrix_sub(&matrix, &other.matrix, &result.matrix);
        return result;
    }

    template<uint32_t cols>
    constexpr Matrix<ROWS, cols> operator*(const Matrix<COLS, cols> &other) const noexcept {
        Matrix<ROWS, cols> result;
        pl::matrix_mul(&matrix, &other.matrix, &result.matrix);
        return result;
    }

    constexpr Matrix operator*(float scale) const noexcept {
        Matrix result;
        pl::matrix_scale(&matrix, scale, &result.matrix);
        return result;
    }

    constexpr friend Matrix<ROWS, COLS> operator*(float scale, const Matrix<ROWS, COLS> &other) noexcept {
        Matrix result;
        pl::matrix_scale(&other.matrix, scale, &result.matrix);
        return result;
    }

    constexpr Matrix operator/(const float scale) const noexcept {
        Matrix result;
        pl::matrix_scale(&matrix, 1.0f / scale, &result.matrix);
        return result;
    }

    [[nodiscard]] constexpr Matrix<COLS, ROWS> transpose() const noexcept {
        Matrix<COLS, ROWS> result;
        pl::matrix_trans(&matrix, &result.matrix);
        return result;
    }

    // constexpr void transpose(Matrix &mat) const requires(is_square()) {
    //     pl::matrix_trans(&matrix, &mat.matrix);
    // }

    constexpr Matrix<COLS, ROWS> operator~() const noexcept {return transpose();}

    constexpr Matrix operator-() const noexcept {return *this * -1.0f;}

    void operator+=(const Matrix &other) noexcept {
        pl::matrix_add(&matrix, &other.matrix, &matrix);
    }

    void operator-=(const Matrix &other) noexcept {
        pl::matrix_sub(&matrix, &other.matrix, &matrix);
    }

    void operator*=(const Matrix<COLS, COLS> &other) noexcept {
        auto result = *this * other;
        *this = result;
        // pl::matrix_mul(&matrix, &other.matrix, &matrix);
    }

    void operator*=(float scale) noexcept {
        pl::matrix_scale(&matrix, scale, &matrix);
    }

    void operator/=(float scale) noexcept {
        pl::matrix_scale(&matrix, 1.0f / scale, &matrix);
    }

    pl::status inv(Matrix &result) noexcept requires(is_square());

    Matrix inv() noexcept requires(is_square());

    template<uint32_t common>
    void assign_multiply(const Matrix<ROWS, common> &first_matrix, const Matrix<common, COLS> &second_matrix) noexcept;
    void assign_add(const Matrix &first_matrix, const Matrix &second_matrix) noexcept;
    void assign_sub(const Matrix &first_matrix, const Matrix &second_matrix) noexcept;
    void assign_scale(float scale) noexcept;
    pl::status assign_inv(Matrix &scr_matrix) noexcept requires(is_square());

// vec
public:

    [[nodiscard]] float norm() const noexcept requires(is_vec()) {
        if constexpr (ROWS == 1) {
            return pl::sqrtf((*this * this->transpose())[0]);
        } else {
            return pl::sqrtf((this->transpose() * *this)[0]);
        }
    };

    [[nodiscard]] Matrix normalized() const noexcept requires(is_vec()) {return (*this / norm());};

    /**
     * @brief 向量差乘 vector cross
     * @param other 另外一个向量 the other vector
     * @return Matrix 相同维度的向量 vector of the same dimension
     */
    Matrix operator^(const Matrix &other) const noexcept requires(is_vec()&&(ROWS==3||COLS==3)) {
        Matrix result;
        const auto &result_array = reinterpret_cast<float*>(result.data);
        const auto &other_array = reinterpret_cast<const float*>(other.data);
        const auto &array = reinterpret_cast<const float*>(data);
        result_array[0] = array[1] * other_array[2] - array[2] * other_array[1];
        result_array[1] = array[2] * other_array[0] - array[0] * other_array[2];
        result_array[2] = array[0] * other_array[1] - array[1] * other_array[0];
        return result;
    }
    // point
    explicit Matrix(float other) noexcept requires(is_point()) : Matrix() {data[0][0] = other;};

// colVec
public:
    constexpr explicit Matrix(const float (&arr)[ROWS]) noexcept requires(is_col()) : Matrix() {
        memcpy(this->data, arr, ROWS * sizeof(float));
    }

    constexpr explicit Matrix(const std::array<float, ROWS> &arr) noexcept requires(is_col()): Matrix() {
        memcpy(this->data, arr.data(), COLS * sizeof(float));
    }

    constexpr float operator*(const Matrix &other) const noexcept requires(is_col()) {
        Matrix<1, 1> result;
        Matrix<1, ROWS> tmp = this->transpose();
        pl::matrix_mul(&tmp.matrix, &other.matrix, &result.matrix);
        return result[0];
    }

    float &operator[](uint32_t num) noexcept requires(is_col()) { return this->data[num][0]; };

    constexpr float operator[](uint32_t num) const noexcept requires(is_col()) { return this->data[num][0]; };

    float &operator()(uint32_t num) noexcept requires(is_col()) { return this->data[num - 1][0]; };

    constexpr float operator()(uint32_t num) const noexcept requires(is_col()) { return this->data[num - 1][0]; };

// rowVec
public:
    constexpr explicit Matrix(const float (&arr)[COLS]) noexcept requires(is_row()): Matrix() {
        memcpy(this->data, arr, COLS * sizeof(float));
    }

    constexpr explicit Matrix(const std::array<float, COLS> &arr) noexcept requires(is_row()): Matrix() {
        memcpy(this->data, arr.data(), COLS * sizeof(float));
    }

    constexpr float operator*(const Matrix &other) const noexcept requires(is_row()) {
        Matrix<1, 1> result;
        Matrix<COLS, 1> tmp = other.transpose();
        pl::matrix_mul(&matrix, &tmp.matrix, &result.matrix);
        return result(1, 1);
    }

    float &operator[](uint32_t num) noexcept requires(is_row() || is_point()) { return this->data[0][num]; };

    constexpr float operator[](uint32_t num) const noexcept requires(is_row() || is_point()) { return this->data[0][num]; };

    float &operator()(uint32_t num) noexcept requires(is_row() || is_point()) { return this->data[0][num - 1]; };

    constexpr float operator()(uint32_t num) const noexcept requires(is_row() || is_point()) { return this->data[0][num - 1]; };

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

template<uint32_t ROWS, uint32_t COLS, mathPl pl>
pl::status Matrix<ROWS, COLS, pl>::assign_inv(Matrix &scr_matrix) noexcept requires(is_square()) {
    typename pl::status ret = pl::matrix_inverse(&scr_matrix.matrix, &matrix);
    return ret;
}

template<uint32_t ROWS, uint32_t COLS, mathPl pl>
void Matrix<ROWS, COLS, pl>::assign_scale(float scale) noexcept {
    pl::matrix_scale(&matrix, scale, &matrix);
}

template<uint32_t ROWS, uint32_t COLS, mathPl pl>
void Matrix<ROWS, COLS, pl>::assign_sub(const Matrix &first_matrix, const Matrix &second_matrix) noexcept {
    pl::matrix_sub(&first_matrix.matrix, &second_matrix.matrix, &matrix);
}

template<uint32_t ROWS, uint32_t COLS, mathPl pl>
void Matrix<ROWS, COLS, pl>::assign_add(const Matrix &first_matrix, const Matrix &second_matrix) noexcept {
    pl::matrix_add(&first_matrix.matrix, &second_matrix.matrix, &matrix);
}

template<uint32_t ROWS, uint32_t COLS, mathPl pl>
template<uint32_t common>
void Matrix<ROWS, COLS, pl>::assign_multiply(const Matrix<ROWS, common> &first_matrix,
                                             const Matrix<common, COLS> &second_matrix) noexcept {
    pl::matrxi_mul(&first_matrix.matrix, &second_matrix.matrix, &matrix);
}

template<uint32_t ROWS, uint32_t COLS, mathPl pl>
inline pl::status Matrix<ROWS, COLS, pl>::inv(Matrix &result) noexcept requires(is_square()) {
    arm_matrix_instance_f32 matrix_bak;
    matrix_bak.numRows = ROWS;
    matrix_bak.numCols = COLS;
    float data_bak[ROWS][COLS]{};
    memcpy(data_bak, data, ROWS * COLS * sizeof(float));
    matrix_bak.pData = reinterpret_cast<float *>(data_bak);
    typename pl::status ret = pl::matrix_inv(&matrix_bak, &result.matrix);
    return ret;
}

template<uint32_t ROWS, uint32_t COLS, mathPl pl>
inline Matrix<ROWS, COLS, pl> Matrix<ROWS, COLS, pl>::inv() noexcept requires(is_square()) {
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

template<uint32_t ROWS, mathPl pl = Windows>
using ColVec = Matrix<ROWS, 1, pl>;

template<uint32_t COLS, mathPl pl = Windows>
using RowVec = Matrix<1, COLS, pl>;
