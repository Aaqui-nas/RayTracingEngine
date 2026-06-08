#pragma once
#include "vec3.h"

class Ray {
public:
    Vec3d origin;
    Vec3d direction;

    Ray(Vec3d origin, Vec3d direction) : origin(origin), direction(direction) {}

    // TODO: maintenant que les opérateurs existent, simplifie at() en une ligne
    Vec3d at(double t) const {
        return origin+t*direction;
    }
};
