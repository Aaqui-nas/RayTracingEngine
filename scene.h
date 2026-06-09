#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <iostream>
#include "shape.h"

namespace rt {
    class Scene {
    public:
        std::vector<std::shared_ptr<Shape>> objects;

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

        std::optional<std::pair<HitRecord, int>> hit(const Ray& ray, double tmin, double tmax) const {
            double closest = tmax;
            std::optional<std::pair<HitRecord, int>> result;

            for (int i = 0; i < (int)objects.size(); i++) {
                if (auto rec = objects[i]->hit(ray, tmin, closest)) {
                    closest = rec->t;
                    result = {*rec, i};
                }
            }
            return result;
        }
    };
}
