#pragma once
#include <map>
#include <string>
#include <random>
#include "geometry/shape.h"

namespace rt {

    inline thread_local std::mt19937 rng(std::random_device{}());
    inline thread_local std::uniform_real_distribution<double> rng_dist(-1.0, 1.0);
    inline thread_local std::uniform_real_distribution<double> rng_dist01(0.0, 1.0);

    inline Vec3d random_in_unit_disk() {
        Vec3d p;
        do { p = Vec3d(rng_dist(rng), rng_dist(rng), 0); } while (dot(p, p) >= 1.0);
        return p;
    }

    std::map<std::string, Material> build_materials();

}
