#pragma once
#include "shape.h"
#include "vec3.h"
#include <map>
#include <string>
#include <random>

namespace rt {
    inline thread_local std::mt19937 rng(std::random_device{}());
    inline thread_local std::uniform_real_distribution<double> rng_dist(-1.0, 1.0);
    inline thread_local std::uniform_real_distribution<double> rng_dist01(0.0, 1.0);

    std::map<std::string, Material> build_materials();
}
