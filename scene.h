#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <iostream>
#include "shape.h"
#include "bvh.h"

namespace rt {
    class Scene {
    public:
        std::vector<std::shared_ptr<Shape>> objects;
        std::shared_ptr<BVHNode> bvh_root;

        Scene() = default;

        Scene(Scene&& other) noexcept
            : objects(std::move(other.objects))
        {
            std::cout << "Move constructor Scene\n";
        }

        Scene& operator=(Scene&& other) noexcept {
            objects = std::move(other.objects);
            return *this;
        }

        void add(std::shared_ptr<Shape>&& shape) {
            objects.push_back(std::move(shape));
        }

        void build_bvh() {
            std::vector<std::shared_ptr<Shape>> bounded;
            std::vector<std::shared_ptr<Shape>> unbounded;

            for (auto& obj : objects) {
                if (obj->bounding_box().has_value())
                    bounded.push_back(obj);
                else
                    unbounded.push_back(obj);
            }

            objects = unbounded;  // garde seulement les infinis dans objects
            if (!bounded.empty())
                bvh_root = std::make_shared<BVHNode>(bounded, 0, bounded.size());
        }

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const {
            std::optional<HitRecord> best;
            double closest = tmax;

            // Tester les objets infinis
            for (int i = 0; i < (int)objects.size(); i++) {
                if (auto h = objects[i]->hit(ray, tmin, closest)) {
                    closest = h->t;
                    best = h;
                }
            }

            // Tester le BVH
            if (bvh_root) {
                if (auto h = bvh_root->hit(ray, tmin, closest)) {
                    best = h;
                }
            }

            if (!best) return std::nullopt;
            return best;
        }
    };
}
