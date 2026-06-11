#pragma once
#include <vector>
#include <memory>
#include <optional>
#include "geometry/shape.h"
#include "geometry/bvh.h"
#include "geometry/sphere.h"
#include "materials/env_map.h"
#include "lights/lights.h"
#include "geometry/concepts.h"
#include "core/sphere_accel.h"

namespace rt {

    class Scene {
    public:
        std::vector<std::shared_ptr<Shape>> objects;
        BVHNode*                  bvh_root = nullptr;
        std::unique_ptr<BVHPool>  bvh_pool;
        std::shared_ptr<EnvMap>   env_map;
        LightList   lights;
        SphereAccel sphere_accel;

        Scene() = default;

        Scene(Scene&& other) noexcept
            : objects(std::move(other.objects))
            , bvh_root(other.bvh_root)
            , bvh_pool(std::move(other.bvh_pool))
            , env_map(std::move(other.env_map))
            , lights(std::move(other.lights))
        { other.bvh_root = nullptr; }

        Scene& operator=(Scene&& other) noexcept {
            objects   = std::move(other.objects);
            bvh_root  = other.bvh_root;
            bvh_pool  = std::move(other.bvh_pool);
            env_map   = std::move(other.env_map);
            lights    = std::move(other.lights);
            other.bvh_root = nullptr;
            return *this;
        }

        void add(std::shared_ptr<Shape> shape) {
            if (auto sphere = std::dynamic_pointer_cast<Sphere>(shape))
                sphere_accel.add(sphere);
            else
                objects.push_back(std::move(shape));
        }

        void build_bvh() {
            std::vector<std::shared_ptr<Shape>> bounded, unbounded;

            std::ranges::partition_copy(objects,
                std::back_inserter(bounded),
                std::back_inserter(unbounded),
                [](const auto& obj) { return obj->bounding_box().has_value(); });

            objects = unbounded;
            if (!bounded.empty()) {
                bvh_pool = std::make_unique<BVHPool>((int)bounded.size());
                bvh_root = bvh_pool->build(bounded, 0, (int)bounded.size());
            }
        }

        std::optional<HitRecord> hit(const Ray& ray, double tmin, double tmax) const {
            std::optional<HitRecord> best;
            double closest = tmax;

            if (!sphere_accel.empty()) {
                if (auto h = sphere_accel.hit(ray, tmin, closest)) {
                    closest = h->t;
                    best    = h;
                }
            }

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
