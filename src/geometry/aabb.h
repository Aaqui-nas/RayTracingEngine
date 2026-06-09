#pragma once
#include <cmath>
#include <algorithm>
#include "core/ray.h"

namespace rt {

    struct AABB {
        Vec3d min_pt, max_pt;

        AABB() {}
        AABB(Vec3d mn, Vec3d mx) : min_pt(mn), max_pt(mx) {}

        bool hit(const Ray& ray, double tmin, double tmax) const {
            for (int i : {0,1,2}) {
                double t0 = (min_pt[i] - ray.origin[i]) / ray.direction[i];
                double t1 = (max_pt[i] - ray.origin[i]) / ray.direction[i];
                if (t0 > t1) std::swap(t0, t1);
                tmin = std::max(tmin, t0);
                tmax = std::min(tmax, t1);
                if (tmax <= tmin) return false;
            }
            return true;
        }
    };

    inline AABB surrounding_box(AABB a, AABB b) {
        return AABB(
            Vec3d(std::min(a.min_pt.x, b.min_pt.x),
                  std::min(a.min_pt.y, b.min_pt.y),
                  std::min(a.min_pt.z, b.min_pt.z)),
            Vec3d(std::max(a.max_pt.x, b.max_pt.x),
                  std::max(a.max_pt.y, b.max_pt.y),
                  std::max(a.max_pt.z, b.max_pt.z))
        );
    }
}
