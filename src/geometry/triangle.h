#pragma once
#include <algorithm>
#include "geometry/shape.h"

namespace rt {

    class Triangle : public Shape {
    public:
        Vec3d A, B, C;
        Vec3d normal;
        Vec3d uv_A, uv_B, uv_C;
        Vec3d T;

        Triangle(Vec3d a, Vec3d b, Vec3d c,
                 Vec3d uv_a, Vec3d uv_b, Vec3d uv_c,
                 Material mat)
            : Shape(mat), A(a), B(b), C(c), uv_A(uv_a), uv_B(uv_b), uv_C(uv_c)
        {
            normal = (B - A).cross(C - A).normalized();
            Vec3d dPos1 = B - A;
            Vec3d dUV1 = uv_B - uv_A - (Vec3d(0, 0, 1)*(uv_B.z - uv_A.z));
            Vec3d dPos2 = C - A;
            Vec3d dUV2 = uv_C - uv_A - (Vec3d(0, 0, 1)*(uv_C.z - uv_A.z));
            double det = dUV1.x * dUV2.y - dUV2.x * dUV1.y;
            T = (dUV2.y * dPos1 - dUV1.y * dPos2) / det;
            T = (T - dot(T, normal) * normal).normalized();
        }


        Triangle(Vec3d a, Vec3d b, Vec3d c, Material mat)
            : Triangle(a, b, c, Vec3d(0,0,0), Vec3d(1,0,0), Vec3d(0,1,0), mat) {};

        Triangle() : Triangle(Vec3d(0,0,0), Vec3d(0,0,0), Vec3d(0,0,0), Material{}) {};

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const override {
            Vec3d E1 = B - A;
            Vec3d E2 = C - A;
            Vec3d h  = ray.direction.cross(E2);
            double det = dot(E1, h);
            if (std::abs(det) < 1e-8) return std::nullopt;

            double f = 1.0 / det;
            Vec3d s  = ray.origin - A;
            double u = f * dot(s, h);
            if (u < 0 || u > 1) return std::nullopt;

            Vec3d q  = s.cross(E1);
            double v = f * dot(ray.direction, q);
            if (v < 0 || (u + v) > 1) return std::nullopt;

            double t = f * dot(E2, q);
            if (t < tmin || t > tmax) return std::nullopt;

            Vec3d point  = ray.at(t);
            Vec3d uv_hit = (1 - u - v) * uv_A + u * uv_B + v * uv_C;

            return HitRecord{
                t, point,
                (det > 0) ? normal : -normal,
                (det > 0),
                this->material, this->emission,
                uv_hit.x, uv_hit.y,
                T
            };
        }

        std::optional<AABB> bounding_box() const override {
            Vec3d mn(std::min({A.x, B.x, C.x}),
                     std::min({A.y, B.y, C.y}),
                     std::min({A.z, B.z, C.z}));
            Vec3d mx(std::max({A.x, B.x, C.x}),
                     std::max({A.y, B.y, C.y}),
                     std::max({A.z, B.z, C.z}));
            Vec3d eps(1e-4, 1e-4, 1e-4);
            return AABB(mn - eps, mx + eps);
        }
    };

}
