#pragma once
#include <optional>
#include <cmath>
#include "shape.h"
#include "vec3.h"

namespace rt {
    class Sphere : public Shape {
    public:
        Vec3d  center;
        double radius;

        Sphere(Vec3d center, double radius, Material mat) : center(center), radius(radius) {
            this->material = mat;
        }

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const override {
            Vec3d oc = ray.origin - center;
            double a = dot(ray.direction, ray.direction);
            double b = 2 * dot(ray.direction, oc);
            double c = dot(oc, oc) - radius * radius;
            double discriminant = b * b - 4 * a * c;
            if (discriminant < 0) return std::nullopt;
            double t = (-b - std::sqrt(discriminant)) / (2 * a);
            if (t < tmin) t = (-b + std::sqrt(discriminant)) / (2 * a);
            if (t < tmin || t > tmax) return std::nullopt;
            Vec3d point = ray.origin + t * ray.direction;
            Vec3d outward_normal = (point - center) / radius;
            bool front_face = dot(ray.direction, outward_normal) < 0;
            return HitRecord{t, point, front_face ? outward_normal : -outward_normal, front_face};
        }
    };
}

