#pragma once
#include <cmath>
#include <algorithm>
#include <ostream>

namespace rt {
    constexpr double pi = 3.14159265358979323846;

    template<typename T>
    class Vec3 {
    public:
        T x, y, z;

        Vec3() : x(0), y(0), z(0) {}
        Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

        T length_squared() const { return x*x + y*y + z*z; }
        double length() const { return std::sqrt(length_squared()); }
        Vec3 normalized() const { return *this / length(); }

        Vec3 operator*(double b) const { return Vec3(x*b, y*b, z*b); }
        Vec3 operator/(double b) const { return Vec3(x/b, y/b, z/b); }

        Vec3 cross(const Vec3& b) const {
            return Vec3(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x);
        }

        Vec3 operator-() const { return Vec3(-x, -y, -z); }

        Vec3& operator+=(const Vec3& b) { x+=b.x; y+=b.y; z+=b.z; return *this; }
        Vec3& operator-=(const Vec3& b) { x-=b.x; y-=b.y; z-=b.z; return *this; }
        Vec3& operator*=(double b)      { x*=b;   y*=b;   z*=b;   return *this; }
        Vec3& operator/=(double b)      { x/=b;   y/=b;   z/=b;   return *this; }

        T operator[](int i) const {
            if (i == 0) return x;
            if (i == 1) return y;
            return z;
        }

        T& operator[](int i) {
            if (i == 0) return x;
            if (i == 1) return y;
            return z;
        }
    };

    template<typename T>
    inline Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b) {
        return Vec3<T>(a.x+b.x, a.y+b.y, a.z+b.z);
    }

    template<typename T>
    inline Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b) {
        return Vec3<T>(a.x-b.x, a.y-b.y, a.z-b.z);
    }

    template<typename T>
    inline Vec3<T> operator*(double b, const Vec3<T>& v) { return v * b; }

    template<typename T>
    inline bool operator==(const Vec3<T>& a, const Vec3<T>& b) { return a.x==b.x && a.y==b.y && a.z==b.z;}

    template<typename T>
    inline Vec3<T> mul(const Vec3<T>& a, const Vec3<T>& b) {
        return Vec3<T>(a.x*b.x, a.y*b.y, a.z*b.z);
    }

    template<typename T>
    inline double dot(const Vec3<T>& a, const Vec3<T>& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }

    template<typename T>
    inline std::ostream& operator<<(std::ostream& os, const Vec3<T>& v) {
        os << v.x << " " << v.y << " " << v.z << " ";
        return os;
    }

    template<typename T>
    std::enable_if_t<std::is_floating_point_v<T>, Vec3<uint8_t>>
    to_unit_range(const Vec3<T>& v) {
        double x = std::clamp(v.x,static_cast<T>(0),static_cast<T>(1))*255;
        double y = std::clamp(v.y,static_cast<T>(0),static_cast<T>(1))*255;
        double z = std::clamp(v.z,static_cast<T>(0),static_cast<T>(1))*255;
        return Vec3(static_cast<uint8_t>(x),static_cast<uint8_t>(y),static_cast<uint8_t>(z));
    }

    using Vec3d = Vec3<double>;
    using Vec3f = Vec3<float>;
    using Vec3i = Vec3<int>;
}
