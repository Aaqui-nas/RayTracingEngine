#pragma once
#include <optional>
#include <cmath>
#include "shape.h"
#include "vec3.h"

namespace rt{
    class Plane : public Shape {
    public:
        Vec3d point;
        Vec3d normal;

        Plane(Vec3d point, Vec3d normal, Material mat) : Shape(mat), point(point), normal(normal) {
        }

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const override {
            double denom = dot(this->normal, ray.direction);
            if (std::abs(denom) < 1e-8) return std::nullopt;
            double t = dot(this->normal, this->point - ray.origin) / denom;
            if (t < tmin || t > tmax) return std::nullopt;
            Vec3d hit_point = ray.origin + t * ray.direction;
            bool front_face = denom < 0;
            Vec3d hit_normal = front_face ? this->normal.normalized() : -(this->normal.normalized());
            return HitRecord{t, hit_point, hit_normal, front_face, this->material, this->emission};
        }
    };
}
