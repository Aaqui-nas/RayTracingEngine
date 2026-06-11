#pragma once
#include <cmath>
#include <type_traits>
#include "core/vec3.h"

namespace rt {

    template<typename T>
    struct shape_traits {
        static constexpr bool is_closed = false;
        static constexpr bool is_convex = false;
    };

    template<typename T, typename = void>
    struct has_bounding_box : std::false_type {};

    template<typename T>
    struct has_bounding_box<T, std::void_t<decltype(std::declval<const T&>().bounding_box())>>
        : std::true_type {};

    struct CylinderTag {};
    struct ConeTag {};

    inline Vec3d compute_normal(const Vec3d& p, double radius, double height, CylinderTag) {
        constexpr double eps = 1e-6;
        if (p.y < eps)          return Vec3d(0.0, -1.0, 0.0);
        if (p.y > height - eps) return Vec3d(0.0,  1.0, 0.0);
        double inv = radius > 0.0 ? 1.0 / radius : 1.0;
        return Vec3d(p.x * inv, 0.0, p.z * inv);
    }

    inline Vec3d compute_normal(const Vec3d& p, double half_angle, double height, ConeTag) {
        constexpr double eps = 1e-6;
        if (p.y > height - eps) return Vec3d(0.0, 1.0, 0.0);
        double dtan = std::tan(half_angle);
        return Vec3d(p.x, -dtan * dtan * p.y, p.z);
    }

}
