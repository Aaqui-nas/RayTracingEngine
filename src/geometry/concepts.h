#pragma once
#include "core/ray.h"
#include "geometry/shape.h"
#include <optional>
#include <ranges>

namespace rt {
    template<typename T>
    concept Hittable = requires(const T& t, const Ray& r, double tmin, double tmax) {
        { t.hit(r, tmin, tmax) } -> std::same_as<std::optional<HitRecord>>;
    };

    template<typename T>
    concept Bounded = Hittable<T> && requires(const T& t) {
        { t.bounding_box() } -> std::same_as<std::optional<AABB>>;
    };

    template<typename R>
    concept HittableRange = std::ranges::range<R> && requires(R r) {
        requires Hittable<std::remove_cvref_t<decltype(*std::ranges::begin(r))>>;
    };

    template<HittableRange R>
    inline std::optional<HitRecord> closest_hit(const R& objects, const Ray& ray,
                                        double tmin, double tmax) {
        std::optional<HitRecord> best;
        double closest = tmax;

        for (auto& obj : objects) {
            if (auto h = obj.hit(ray, tmin, closest)) {
                closest = h->t;
                best    = h;
            }
        }
        return best;
    }

    inline auto emissive_shapes(const std::vector<std::shared_ptr<Shape>>& objects) {
        return objects | std::views::filter([](const auto& obj) {
            return obj->emission.length() > 0.0;
        });
    }

    inline auto bounded_shapes(const std::vector<std::shared_ptr<Shape>>& objects) {
        return objects | std::views::filter([](const auto& obj) {
            return obj->bounding_box() != std::nullopt;
        });
    }

    inline void sort_by_axis(std::vector<std::shared_ptr<Shape>>& objects, int axis) {
        std::ranges::sort(objects, [axis](const auto& a, const auto& b) {
            auto ba = a->bounding_box();
            auto bb = b->bounding_box();
            if (!ba || !bb) return false;
            double ca = (ba->min_pt[axis] + ba->max_pt[axis]) * 0.5;
            double cb = (bb->min_pt[axis] + bb->max_pt[axis]) * 0.5;
            return ca < cb;
        });
    }

    template<typename T>
    inline void register_shape(T& shape) {
        static_assert(Hittable<T>,
            "register_shape() requires a type with hit(Ray, double, double) -> optional<HitRecord>");
    }

    template<typename T>
    concept Renderable = Hittable<T> && requires(const T& t) {
        { t.material } -> std::convertible_to<Material>;
    };

    template<Renderable T>
    inline void scatter_at(const T& shape, const Ray& ray, const HitRecord& rec) {
        auto scatter = shape.material(ray, rec);
        // ...
    }

    inline std::optional<HitRecord> closest_hit_ptrs(
        const std::vector<std::shared_ptr<Shape>>& objects,
        const Ray& ray, double tmin, double tmax) {
            std::optional<HitRecord> best;
            double closest = tmax;

            for (auto& obj : objects) {
                if (auto h = obj->hit(ray, tmin, closest)) {
                    closest = h->t;
                    best    = h;
                }
            }
            return best;
        }

    template<Bounded T>
    inline std::optional<AABB> hittable_bbox(const T& shape) {
        return shape.bounding_box();
    }

}
