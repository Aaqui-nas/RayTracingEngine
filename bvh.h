#pragma once
#include "shape.h"
#include "aabb.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <random>

namespace rt {

    class BVHNode : public Shape {
    public:
        std::shared_ptr<Shape> left;
        std::shared_ptr<Shape> right;
        AABB box;

        BVHNode(std::vector<std::shared_ptr<Shape>>& shapes, int start, int end) : Shape(Material{}) {

            int n = end - start;
            switch (n)
            {
            case 1:
                left = right = shapes[start];
                break;
            case 2:
                left = shapes[start], right = shapes[start+1];
                break;
            default:
                int axis = rand() % 3;
                std::sort(shapes.begin()+start, shapes.begin()+end,
                    [axis](const auto& a, const auto& b) {
                    return a->bounding_box().value().min_pt[axis]
                        < b->bounding_box().value().min_pt[axis];
                    }
                );
                int mid = start + n/2;
                left = std::make_shared<BVHNode>(shapes, start, mid);
                right = std::make_shared<BVHNode>(shapes, mid, end);
                break;
            }
            box = surrounding_box(left->bounding_box().value(), right->bounding_box().value());
        };

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const override {
            if (!box.hit(ray, tmin, tmax)) return std::nullopt;
            auto hit_left = left->hit(ray, tmin, tmax);
            if (hit_left) tmax = hit_left->t;
            auto hit_right = right->hit(ray, tmin, tmax);
            if (hit_right && hit_right->t < tmax) return hit_right;
            return hit_left;
        }

        std::optional<AABB> bounding_box() const override { return box; }
    };

}
