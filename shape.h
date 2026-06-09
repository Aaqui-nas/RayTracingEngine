#pragma once
#include <optional>
#include <functional>
#include "ray.h"
#include "vec3.h"

namespace rt {
    struct HitRecord {
        double t;
        Vec3d point;
        Vec3d normal;
        bool front_face;
    };

    struct Scatter {
        Vec3d attenuation;
        Ray scattered;
    };

    using Material = std::function<std::optional<Scatter>(const Ray&, const HitRecord&)>;

    class Shape {
    public:
        Material material;
        Vec3d emission = Vec3d(0, 0, 0);
        virtual std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const = 0;
        virtual ~Shape() {}
    };
}
