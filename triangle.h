#pragma once
#include "shape.h"

namespace rt {
    class Triangle : public Shape {
    public:
        Vec3d A, B, C;
        Vec3d normal;

        Triangle(Vec3d a, Vec3d b, Vec3d c, Material mat)
            : Shape(mat), A(a), B(b), C(c)
        {
            normal = (B - A).cross(C - A).normalized();
        }

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const override {
            Vec3d E1 = B - A;
            Vec3d E2 = C - A;

            Vec3d h = ray.direction.cross(E2);

            double det = dot(E1, h);
            if (abs(det)<1e-8) return std::nullopt;

            double f = 1.0/det;

            Vec3d s = ray.origin - A;
            double u = f*dot(s,h);
            if (u < 0 || u > 1) return std::nullopt;

            Vec3d q = s.cross(E1);
            double v = f*dot(ray.direction, q);
            if (v < 0 || (u+v) > 1) return std::nullopt;

            double t = f * dot(E2, q);
            if (t < tmin || t > tmax) return std::nullopt;

            Vec3d point = ray.origin + t * ray.direction;
            return HitRecord{t, point, (det > 0) ? normal : -normal, (det > 0)};
        }
    };
}
