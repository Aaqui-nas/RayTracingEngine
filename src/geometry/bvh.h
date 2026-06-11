#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <random>
#include "geometry/shape.h"
#include "geometry/aabb.h"
#include "core/object_pool.h"

namespace rt {

    class BVHNode : public Shape {
    public:
        BVHNode* left_node  = nullptr;
        BVHNode* right_node = nullptr;
        std::shared_ptr<Shape> left_leaf  = nullptr;
        std::shared_ptr<Shape> right_leaf = nullptr;
        AABB bbox;

        BVHNode(BVHNode* left, BVHNode* right, AABB bbox)
            : Shape(Material{}), left_node(left), right_node(right), bbox(bbox){}

        BVHNode(std::shared_ptr<Shape> left, std::shared_ptr<Shape> right, AABB bbox)
            : Shape(Material{}), left_leaf(left), right_leaf(right), bbox(bbox){}

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const override {
            if (!bbox.hit(ray, tmin, tmax)) return std::nullopt;
            std::optional<HitRecord> hit_left;
            std::optional<HitRecord> hit_right;
            if(left_node != nullptr) hit_left = left_node->hit(ray, tmin, tmax);
            else hit_left = left_leaf->hit(ray, tmin, tmax);
            if (hit_left) tmax = hit_left->t;
            if(right_node != nullptr) hit_right = right_node->hit(ray, tmin, tmax);
            else hit_right = right_leaf->hit(ray, tmin, tmax);
            if (hit_right && hit_right->t < tmax) return hit_right;
            return hit_left;
        }

        std::optional<AABB> bounding_box() const override { return bbox; }
    };

    class BVHPool {
        ObjectPool<BVHNode> pool;

    public:
        explicit BVHPool(int max_objects)
            : pool(2 * max_objects) {}

        BVHNode* build(std::vector<std::shared_ptr<Shape>>& shapes, int start, int end) {
            int n = end - start;
            switch (n) {
                case 1:
                    return pool.construct(shapes[start],shapes[start], shapes[start]->bounding_box().value());
                case 2:
                    return pool.construct(shapes[start],shapes[start + 1], surrounding_box(shapes[start]->bounding_box().value(),shapes[start+1]->bounding_box().value()));
                default: {
                    int axis = rand() % 3;
                    std::sort(shapes.begin() + start, shapes.begin() + end,
                        [axis](const auto& a, const auto& b) {
                            return a->bounding_box().value().min_pt[axis]
                                    < b->bounding_box().value().min_pt[axis];
                        });
                    int mid = start + n / 2;
                    BVHNode* left = build(shapes, start, mid);
                    BVHNode* right = build(shapes, mid, end);
                    return pool.construct(left,right,surrounding_box(left->bbox,right->bbox));
                }
            }
        }
    };

}


