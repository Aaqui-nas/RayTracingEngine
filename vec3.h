#pragma once
#include <cmath>
#include <ostream>

// TODO: transforme cette classe en template Vec3<T>
template<typename T>
class Vec3 {
public:
    T x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    double length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const { return *this / length(); }

    Vec3 operator*(double b) const{
        return Vec3(x*b, y*b, z*b);
    }

    Vec3 operator/(double b) const{
        return Vec3(x/b, y/b, z/b);
    }

    Vec3 cross(const  Vec3& b) const {
        return Vec3(y*b.z-z*b.y, z*b.x-x*b.z, x*b.y-y*b.x);
    }

    Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    // TODO: opérateurs d'affectation composés (+=, -=, *=, /=)
    Vec3& operator+=(const Vec3& b){
        x+=b.x; y+=b.y; z+=b.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& b){
        x-=b.x; y-=b.y; z-=b.z;
        return *this;
    }

    Vec3& operator*=(const double& b){
        x*=b; y*=b; z*=b;
        return *this;
    }

    Vec3& operator/=(const double& b){
        x/=b; y/=b; z/=b;
        return *this;
    }


};

// TODO: opérateurs (à adapter pour Vec3<T>)
template<typename T>
inline Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b){
    return Vec3(a.x+b.x, a.y+b.y, a.z+b.z);
}

template<typename T>
inline Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b){
    return Vec3(a.x-b.x, a.y-b.y, a.z-b.z);
}

template<typename T>
inline Vec3<T> operator*(double b, const Vec3<T>& v){
        return v*b;
}

template<typename T>
inline double dot(const Vec3<T>& a, const  Vec3<T>& b){
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Vec3<T>& v){
    os << v.x << " " << v.y << " " << v.z << " ";
    return os;
}

// TODO: aliases Vec3d, Vec3f, Vec3i
using Vec3d = Vec3<double>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;
