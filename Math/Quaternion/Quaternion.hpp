//
// Created by liaohy on 25-3-13.
//

#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include "../Matrix/Matrix.hpp"

template<mathPl pl = Windows>
class Quaternion {
public:
    template<uint32_t T>
    using Vec = ColVec<T, pl>;

    static constexpr Quaternion zero() { return Quaternion{0, 0, 0, 0}; }

    static constexpr Quaternion init() { return Quaternion{1, 0, 0, 0}; }

public:
    constexpr Quaternion(float w, float x, float y, float z) : w(w), u(Vec<3>(
            {x, y, z})) {};

    constexpr Quaternion(float w, const Vec<3> &u) : w(w), u(u) {};

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
        return Quaternion{-w, -u[0], -u[1], -u[2]};
    }

    constexpr Quaternion conj() const {
        return Quaternion{w, -u[0], -u[1], -u[2]};
    }

    constexpr Quaternion operator~() const {
        return Quaternion{w, -u[0], -u[1], -u[2]};
    }

    constexpr Quaternion normalized() const {
        auto size = w * w + (u * u);
        if (size < 1e-7) { return zero(); }
        return Quaternion{w / size, u / size};
    }

    constexpr Quaternion inv() const {
        auto size = w * w + (u * u);
        if (size < 1e-7) { return zero(); }
        return Quaternion{w / size, -u / size};
    }

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

template<mathPl pl = Windows>
class UnitQuat : public Quaternion<pl> {
public:
    template<uint32_t T>
    using Vec = Quaternion<pl>::template Vec<T>;

    constexpr UnitQuat(float theta, const Vec<3> &vec) : Quaternion<pl>(pl::cos(theta / 2), pl::sin(theta / 2) * vec) {};
    constexpr explicit UnitQuat(const Quaternion<pl>& q) : Quaternion<pl>(q) {};

    constexpr UnitQuat inv() const {return UnitQuat(this->conj());}

    void update(float theta) {
        this->w = pl::cos(theta / 2);
        this->u = pl::sin(theta / 2) * this->Spin;
    };

};


#endif //QUATERNION_HPP
