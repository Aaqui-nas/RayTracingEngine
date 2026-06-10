#pragma once
#include <vector>
#include <memory>
#include <optional>
#include "core/simd.h"
#include "geometry/sphere.h"

namespace rt {

class SphereAccel {
    struct Entry {
        SphereBlock block;
        std::shared_ptr<Sphere> spheres[4];
        int count = 0;
    };

    std::vector<Entry> entries;

public:
    void add(std::shared_ptr<Sphere> s) {
        if (entries.empty() || entries.back().count == 4)
            entries.emplace_back();

        Entry& e = entries.back();
        int k = e.count;
        e.block.cx[k] = s->center.x;
        e.block.cy[k] = s->center.y;
        e.block.cz[k] = s->center.z;
        e.block.r2[k] = s->radius * s->radius;
        e.spheres[k]  = s;
        e.count++;
    }

    bool empty() const { return entries.empty(); }
    int  size()  const {
        if (entries.empty()) return 0;
        return (int(entries.size()) - 1) * 4 + entries.back().count;
    }

    std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const {
        const Sphere* winner  = nullptr;
        double        closest = tmax;
        alignas(32) double t_results[4];

        for (const auto& e : entries) {
            hit_spheres4(ray, e.block, t_results);
            for (int k = 0; k < e.count; k++) {
                double t = t_results[k];
                if (t > tmin && t < closest) {
                    closest = t;
                    winner  = e.spheres[k].get();
                }
            }
        }

        if (!winner) return std::nullopt;
        return winner->hit(ray, tmin, tmax);
    }
};

}
