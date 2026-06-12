#pragma once

#include <cmath>
#include <limits>
#include "core/vec3.h"
#include "core/ray.h"
#include "scene/scene.h"
#include "materials/materials.h"
#include "lights/lights.h"

namespace rt {

struct GBufferSample {
    double t = std::numeric_limits<double>::infinity();
    Vec3d normal{0.0, 0.0, 0.0};
    Vec3d point{0.0, 0.0, 0.0};
};

inline GBufferSample sample_gbuffer(const Ray& ray, const Scene& scene) {
    if (auto rec = scene.hit(ray, 0.001, 1000.0))
        return {rec->t, rec->normal, rec->point};
    return {};
}

inline double clamp(double v, double lo, double hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

inline Vec3d gamma(Vec3d v, double g) {
    v = v / 255;
    return 255 * Vec3d(std::pow(v.x, 1.0/g), std::pow(v.y, 1.0/g), std::pow(v.z, 1.0/g));
}

inline Vec3d ray_color(const Ray& ray, const Scene& scene, int depth) {
    auto sky = [&]() -> Vec3d {
        if (scene.env_map) {
            Vec3d c = scene.env_map->background(ray.direction.normalized());
            c = c * 2.0;
            return Vec3d(c.x / (1 + c.x), c.y / (1 + c.y), c.z / (1 + c.z)) * 255.0;
        }
        Vec3d norm = ray.direction.normalized();
        double y = (norm.y + 1) / 2;
        return (1-y) * Vec3d(255,255,255) + y * Vec3d(128,178,255);
    };

    if (depth == 0) return sky();

    if (auto rec = scene.hit(ray, 0.001, 1000)) {
        Vec3d emitted = rec->emission;
        auto scatter = rec->material(ray, *rec);
        Vec3d direct(0, 0, 0);
        if (!scene.lights.empty() && scatter) {
            auto ls = scene.lights.sample(rec->point, rng_dist01(rng), rng_dist01(rng), rng_dist01(rng));
            if (ls && ls->pdf > 0) {
                Vec3d shadow_origin = rec->point + 1e-4 * rec->normal;
                Vec3d light_point   = rec->point + ls->direction * ls->distance;
                if (!scene.occluded(shadow_origin, light_point)) {
                    double cos_theta = std::max(0.0, dot(ls->direction.normalized(), rec->normal));
                    double pdf_brdf = cos_theta / pi;
                    // Utilise brdf_eval si disponible (ex. Cook-Torrance) sinon attenuation (Lambertian).
                    Vec3d direct_brdf = scatter->brdf_eval
                        ? scatter->brdf_eval(ls->direction.normalized())
                        : scatter->attenuation;
                    direct = mul(direct_brdf, ls->radiance) * cos_theta / (ls->pdf * pi);
                    direct *= mis_weight(ls->pdf, pdf_brdf);
                }
            }
        }

        if (scatter)
            return direct + emitted
                   + mul(scatter->attenuation / 255.0,
                         ray_color(scatter->scattered, scene, depth - 1));
        return direct + emitted;
    }
    return sky();
}

}
