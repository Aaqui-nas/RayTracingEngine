#pragma once
#include <vector>
#include <memory>
#include <optional>
#include "geometry/shape.h"
#include "geometry/bvh.h"
#include "materials/env_map.h"
#include "lights/lights.h"
#include "geometry/concepts.h"

namespace rt {

    class Scene {
    public:
        std::vector<std::shared_ptr<Shape>> objects;
        std::shared_ptr<BVHNode>  bvh_root;
        std::shared_ptr<EnvMap>   env_map;
        LightList lights;

        Scene() = default;

        Scene(Scene&& other) noexcept
            : objects(std::move(other.objects))
            , bvh_root(std::move(other.bvh_root))
            , env_map(std::move(other.env_map))
            , lights(std::move(other.lights))
        {}

        Scene& operator=(Scene&& other) noexcept {
            objects  = std::move(other.objects);
            bvh_root = std::move(other.bvh_root);
            env_map  = std::move(other.env_map);
            lights = std::move(other.lights);
            return *this;
        }

        void add(std::shared_ptr<Shape> shape) {
            objects.push_back(std::move(shape));
        }

        void build_bvh() {
            std::vector<std::shared_ptr<Shape>> bounded, unbounded;

            std::ranges::partition_copy(objects,
                std::back_inserter(bounded),
                std::back_inserter(unbounded),
                [](const auto& obj) { return obj->bounding_box().has_value(); });

            objects = unbounded;
            if (!bounded.empty())
                bvh_root = std::make_shared<BVHNode>(bounded, 0, (int)bounded.size());
        }

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const {
            std::optional<HitRecord> best;
            double closest = tmax;

            for (auto& obj : objects) {
                if (auto h = obj->hit(ray, tmin, closest)) {
                    closest = h->t;
                    best    = h;
                }
            }

            if (bvh_root) {
                if (auto h = bvh_root->hit(ray, tmin, closest))
                    best = h;
            }

            return best;
        }

        bool occluded(const Vec3d& from, const Vec3d& to) const {
            Ray ray = Ray(from, (to-from).normalized());
            return hit(ray, 1e-4, (to-from).length()-1e-3).has_value();
        }

    };
}
