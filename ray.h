#pragma once
#include "vec3.h"

namespace rt {
    class Ray {
    public:
        Vec3d origin;
        Vec3d direction;

        Ray(Vec3d origin, Vec3d direction) : origin(origin), direction(direction) {}

        Vec3d at(double t) const {
            return origin+t*direction;
        }
    };
}
