#pragma once
#include "ray.h"
#include <cmath>

namespace rt {
    constexpr double pi = 3.14159265358979323846;

    class Camera {
    public:
        Vec3d origin = Vec3d(0, 0, 0);
        Vec3d lower_left_corner;
        Vec3d horizontal;
        Vec3d vertical;

        Camera(double vfov_deg, double aspect_ratio) {
            double half_height = tan((vfov_deg* pi / 180.0) / 2);
            double half_width = aspect_ratio*half_height;
            horizontal = Vec3d(2* half_width, 0, 0);
            vertical = Vec3d(0, 2*half_height, 0);
            lower_left_corner = origin - horizontal/2 - vertical/2 - Vec3d(0,0,1);
        } ;

        Camera() : Camera(96.74, 4.0/2.25) {};

        Ray get_ray(double u, double v) const {
            return Ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
        }
    };
}
