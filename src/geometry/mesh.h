#pragma once
#include <vector>
#include "geometry/triangle.h"

namespace rt {

    class Mesh : public Shape {
    public:
        std::vector<Triangle> triangles;

        Mesh(Material mat) : Shape(mat) {}

        void add(Triangle t) { triangles.push_back(t); }

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const override {
            double closest = tmax;
            std::optional<HitRecord> result;
            for (const Triangle& tri : triangles) {
                if (auto rec = tri.hit(ray, tmin, closest)) {
                    closest = rec->t;
                    result  = *rec;
                }
            }
            return result;
        }

        std::optional<AABB> bounding_box() const override {
            if (triangles.empty()) return std::nullopt;
            auto box = triangles[0].bounding_box().value();
            for (size_t i = 1; i < triangles.size(); i++)
                box = surrounding_box(box, triangles[i].bounding_box().value());
            return box;
        }

        void translate(const Vec3d& offset) {
            for (auto& tri : triangles) {
                tri.A += offset;
                tri.B += offset;
                tri.C += offset;
            }
        }
    };

}
