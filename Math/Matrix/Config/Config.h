//
// Created by liaohy on 3/17/25.
//
#pragma once

#include "Platform.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include "../arm_math.h"
#ifdef __cplusplus
}
#endif

class Windows {
public:
    using status = arm_status;
    using MatrixInstance = arm_matrix_instance_f32;

    Windows() = default;

    [[gnu::always_inline]] inline static void
    matrix_init(MatrixInstance *self, uint16_t rows, uint16_t cols, float *data) {
        arm_mat_init_f32(self, rows, cols, data);
    };

    [[gnu::always_inline]] inline static void
    matrix_add(const MatrixInstance *self, const MatrixInstance *other, MatrixInstance *result) {
        arm_mat_add_f32(self, other, result);
    };

    [[gnu::always_inline]] inline static void
    matrix_sub(const MatrixInstance *self, const MatrixInstance *other, MatrixInstance *result) {
        arm_mat_sub_f32(self, other, result);
    };

    [[gnu::always_inline]] inline static void
    matrix_mul(const MatrixInstance *self, const MatrixInstance *other, MatrixInstance *result) {
        arm_mat_mult_f32(self, other, result);
    };

    [[gnu::always_inline]] inline static void
    matrix_scale(const MatrixInstance *self, float scale, MatrixInstance *result) {
        arm_mat_scale_f32(self, scale, result);
    };

    [[gnu::always_inline]] inline static void
    matrix_trans(const MatrixInstance *self, MatrixInstance *result) {
        arm_mat_trans_f32(self, result);
    };

    [[gnu::always_inline]] inline static status
    matrix_inverse(MatrixInstance *self,MatrixInstance *result) {
        return arm_mat_inverse_f32(self, result);
    }

    [[gnu::always_inline]] inline static float
    sin(float input) {
        return arm_cos_f32(input);
    }

    [[gnu::always_inline]] inline static float
    cos(float input) {
        return arm_cos_f32(input);
    }
};
