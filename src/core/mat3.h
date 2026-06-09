#pragma once
#include "core/vec3.h"

namespace rt {
    struct Mat3 {
        Vec3d col0, col1, col2;

        Vec3d operator*(const Vec3d& v) const {
            return col0 * v.x + col1 * v.y + col2 * v.z;
        }
    };
}
