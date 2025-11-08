//
// Created by liaohy on 25-3-13.
//

#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include "../Matrix/Matrix.hpp"


namespace quaternion_dep {
    template<uint32_t T>
    using Vec = RowVec<T>;

    template<bool Unit>
    struct UnitQ;

    template<>
    struct UnitQ<true> {
        Vec<3> Spin;
    };

    template<>
    struct UnitQ<false> {
    };
}

template<bool Unit = false>
class Quaternion : public quaternion_dep::UnitQ<Unit> {
public:
    template<uint32_t T>
    using Vec = quaternion_dep::Vec<T>;

    static consteval Quaternion zero() { return Quaternion{0, 0, 0, 0}; }

    static consteval Quaternion init() { return Quaternion{1, 0, 0, 0}; }

public:
    constexpr Quaternion(float w, float x, float y, float z) requires((!Unit)): w(w), u(Vec<3>(
            {x, y, z})) {};

    constexpr Quaternion(float w, float x, float y, float z) requires((Unit))
            : quaternion_dep::UnitQ<Unit>(Vec<3>({x, y, z})), w(w), u(Vec<3>({x, y, z})) {};

    constexpr Quaternion(float w, const Vec<3> &u) requires((Unit)): quaternion_dep::UnitQ<Unit>(u), w(w), u(u) {};

    constexpr Quaternion(float w, const Vec<3> &u) requires((!Unit)): w(w), u(u) {};

    constexpr Quaternion(float theta, const Vec<3> &vec, bool) requires((Unit))
            : quaternion_dep::UnitQ<Unit>(vec), w(Vec<3>::cos(theta / 2)), u(Vec<3>::sin(theta / 2) * vec) {};


    [[nodiscard]] constexpr Quaternion derivative(float wx, float wy, float wz) const {
        return Quaternion(0, wx * 0.5f, wy * 0.5f, wz * 0.5f) * (*this);
    };

    constexpr Quaternion operator*(const Quaternion &other) const {
        return Quaternion{other.w * w - (other.u * u), other.w * u + w * other.u + (u ^ other.u)};
    };

    constexpr Quaternion operator*(float scale) const {
        return Quaternion{scale * w, u * scale};
    };

    friend constexpr Quaternion operator*(float scale, const Quaternion &other) {
        return Quaternion{scale * other.w, other.u * scale};
    };

    constexpr Quaternion operator+(const Quaternion &other) const {
        return Quaternion{w + other.w, u + other.u};
    }

    constexpr Quaternion operator-(const Quaternion &other) const {
        return Quaternion{w - other.w, u - other.u};
    }

    constexpr Quaternion operator-() const {
        return Quaternion{-w, -u(1), -u(2), -u(3)};
    }

    constexpr Quaternion conj() const {
        return Quaternion{w, -u(1), -u(2), -u(3)};
    }

    constexpr Quaternion operator~() const {
        return Quaternion{w, -u(1), -u(2), -u(3)};
    }

    constexpr Quaternion inv() const requires((!Unit)) {
        auto size = w * w + (u * u);
        if (size < 1e-7) { return zero(); }
        return Quaternion{w / size, -u(1) / size, -u(2) / size, -u(3) / size};
    }

    constexpr Quaternion inv() const requires((Unit)) {
        return Quaternion{w, -u(1), -u(2), -u(3)};
    }

    void update(float theta) requires((Unit)) {
        w = Vec<3>::cos(theta / 2);
        u = Vec<3>::sin(theta / 2) * this->Spin;
    };

    template<typename OStream>
    friend OStream &operator<<(OStream &os, const Quaternion &mat) {
        os << "\n[";
        os << " " << mat.w << ",";
        os << " " << mat.u;
        os << " ]";
        return os;
    }

protected:
    float w;
    Vec<3> u;
};


#endif //QUATERNION_HPP
