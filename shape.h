#pragma once
#include <optional>
#include <functional>
#include "ray.h"
#include "vec3.h"
#include "aabb.h"

namespace rt {
    struct Scatter {
        Vec3d attenuation;
        Ray scattered;
    };

    struct HitRecord;

    using Material = std::function<std::optional<Scatter>(const Ray&, const HitRecord&)>;

    struct HitRecord{
        double t;
        Vec3d point;
        Vec3d normal;
        bool front_face;
        Material material;
        Vec3d emission;
    };

    class Shape {
    public:
        Material material;
        Vec3d emission = Vec3d(0, 0, 0);
        Shape(Material mat) : material(mat) {};
        virtual std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const = 0;
        virtual ~Shape() {};
        virtual std::optional<AABB> bounding_box() const {
            return std::nullopt;
        };
    };
}
