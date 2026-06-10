#pragma once
#include <map>
#include <string>
#include <random>
#include "geometry/shape.h"
#include "materials/texture.h"

namespace rt {

    inline thread_local std::mt19937 rng(std::random_device{}());
    inline thread_local std::uniform_real_distribution<double> rng_dist(-1.0, 1.0);
    inline thread_local std::uniform_real_distribution<double> rng_dist01(0.0, 1.0);

    inline Vec3d random_in_unit_disk() {
        Vec3d p;
        do { p = Vec3d(rng_dist(rng), rng_dist(rng), 0); } while (dot(p, p) >= 1.0);
        return p;
    }

    inline Vec3d random_in_unit_sphere() {
        Vec3d p;
        do { p = Vec3d(rng_dist(rng), rng_dist(rng), rng_dist(rng)); } while (dot(p, p) >= 1.0);
        return p;
    }

    inline Vec3d random_unit_vector() {
        Vec3d v = random_in_unit_sphere();
        return v / std::sqrt(dot(v, v));
    }

    Material normal_mapped(Material base, TexturePtr normal_map);

    std::map<std::string, Material> build_materials();

    inline Vec3d reflect(Vec3d v, Vec3d n) {
        return v - 2 * dot(v, n) * n;
    }

}
