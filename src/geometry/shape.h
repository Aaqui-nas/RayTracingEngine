#pragma once
#include <optional>
#include <functional>
#include "core/ray.h"
#include "core/vec3.h"
#include "geometry/aabb.h"

namespace rt {
    struct Scatter {
        Vec3d attenuation;
        Ray   scattered;
        // Pour MIS direct : f_r(L) × 255 × π, même échelle que attenuation.
        // Si null, utilise attenuation (convient pour Lambertian et métal).
        std::function<Vec3d(const Vec3d&)> brdf_eval;
    };

    struct HitRecord;

    using Material = std::function<std::optional<Scatter>(const Ray&, const HitRecord&)>;

    struct HitRecord {
        double   t          = 0.0;
        Vec3d    point      = {};
        Vec3d    normal     = {};
        bool     front_face = false;
        Material material   = {};
        Vec3d    emission   = {};
        double   u = 0.0, v = 0.0;
        Vec3d tangent = {};
    };

    class Shape {
    public:
        Material material;
        Vec3d emission = Vec3d(0, 0, 0);

        Shape(Material mat) : material(mat) {}
        virtual ~Shape() {}

        virtual std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const = 0;
        virtual std::optional<AABB> bounding_box() const { return std::nullopt; }
    };
}
