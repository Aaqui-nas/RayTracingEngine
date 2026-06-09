#pragma once
#include "ray.h"
#include <cmath>
#include "materials.h"

namespace rt {
    constexpr double pi = 3.14159265358979323846;

    inline rt::Vec3d random_in_unit_disk() {
        Vec3d p;
        do { p = Vec3d(rng_dist(rng), rng_dist(rng), 0); } while (dot(p,p) >= 1.0);
        return p;
    }

    class Camera {
    public:
        Vec3d origin = Vec3d(0, 0, 0);
        Vec3d lower_left_corner;
        Vec3d horizontal;
        Vec3d vertical;
        Vec3d u, v;
        double lens_radius;

        Camera(double vfov_deg, double aspect_ratio) {
            double half_height = tan((vfov_deg* pi / 180.0) / 2);
            double half_width = aspect_ratio*half_height;
            horizontal = Vec3d(2* half_width, 0, 0);
            vertical = Vec3d(0, 2*half_height, 0);
            lower_left_corner = origin - horizontal/2 - vertical/2 - Vec3d(0,0,1);
            lens_radius = 0;
        } ;

        Camera(Vec3d from, Vec3d to, Vec3d vup, double vfov_deg, double aspect_ratio, double aperture = 0.0, double focus_dist = 1.0) {
            origin = from;
            Vec3d w = (from-to).normalized();
            u = vup.cross(w).normalized();
            v = w.cross(u);

            double half_h = tan((vfov_deg* pi / 180.0) / 2);
            double half_w = aspect_ratio * half_h;

            lower_left_corner = from - u*half_w*focus_dist - v*half_h*focus_dist - w*focus_dist;
            horizontal = 2*u*half_w*focus_dist;
            vertical = 2*v*half_h*focus_dist;

            lens_radius = aperture/2;
        }

        Camera() : Camera(96.74, 4.0/2.25) {};

        Ray get_ray(double s, double t) const {
            Vec3d rd = lens_radius * random_in_unit_disk();
            Vec3d offset = u*rd.x + v*rd.y;
            return Ray(origin+offset, lower_left_corner + s*horizontal + t*vertical - origin - offset);
        }
    };
}
