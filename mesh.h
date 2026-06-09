#pragma once
#include "triangle.h"
#include <vector>

namespace rt {
    class Mesh : public Shape {
    public:
        std::vector<Triangle> triangles;

        Mesh(Material mat) : Shape(mat) {}

        void add(Triangle t) { triangles.push_back(t); }

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const override {
            double closest = tmax;
            std::optional<HitRecord> result;

            for (const Triangle& triangle : triangles) {
                if (auto rec = triangle.hit(ray, tmin, closest)) {
                    closest = rec->t;
                    result = *rec;
                }

            }
            return result;
        }
    };
}
