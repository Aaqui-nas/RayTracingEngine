#pragma once
#include <optional>
#include <algorithm>
#include <type_traits>
#include <cmath>
#include <limits>
#include <utility>

#include "geometry/shape.h"
#include "core/vec3.h"
#include "geometry/shape_traits.h"

namespace rt {

    struct sorted_corners_t { explicit sorted_corners_t() = default; };
    inline constexpr sorted_corners_t sorted_corners{};

    class Box : public Shape {
        Vec3d pmin, pmax;

        static void order(Vec3d& a, Vec3d& b) {
            for (int i : {0, 1, 2})
                if (a[i] > b[i]) std::swap(a[i], b[i]);
        }

    public:
        template<typename T,
                 std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        Box(const Vec3<T>& lo, const Vec3<T>& hi, Material mat, sorted_corners_t)
            : Shape(std::move(mat)),
              pmin(lo.x, lo.y, lo.z),
              pmax(hi.x, hi.y, hi.z) {}

        template<typename T,
                 std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        Box(const Vec3<T>& a, const Vec3<T>& b, Material mat)
            : Shape(std::move(mat)),
              pmin(a.x, a.y, a.z),
              pmax(b.x, b.y, b.z) {
            order(pmin, pmax);
        }

        std::optional<HitRecord> hit(const Ray& r, double tmin, double tmax) const override {
            constexpr double inf = std::numeric_limits<double>::infinity();

            double t_enter = -inf, t_exit = inf;
            int    enter_axis = 0, exit_axis = 0;
            double enter_sign = -1.0, exit_sign = 1.0;

            for (int i : {0, 1, 2}) {
                double inv    = 1.0 / r.direction[i];
                double t_near = (pmin[i] - r.origin[i]) * inv;
                double t_far  = (pmax[i] - r.origin[i]) * inv;

                double near_sign = -1.0;
                double far_sign  = +1.0;
                if (t_near > t_far) {
                    std::swap(t_near, t_far);
                    std::swap(near_sign, far_sign);
                }

                if (t_near > t_enter) { t_enter = t_near; enter_axis = i; enter_sign = near_sign; }
                if (t_far  < t_exit ) { t_exit  = t_far;  exit_axis  = i; exit_sign  = far_sign;  }
            }

            if (!(t_enter <= t_exit && t_exit > tmin && t_enter < tmax))
                return std::nullopt;

            double t;
            int    axis;
            double sign;
            if (t_enter > tmin) { t = t_enter; axis = enter_axis; sign = enter_sign; }
            else                { t = t_exit;  axis = exit_axis;  sign = exit_sign;  }

            if (t < tmin || t > tmax)
                return std::nullopt;

            Vec3d point(r.origin.x + t * r.direction.x,
                        r.origin.y + t * r.direction.y,
                        r.origin.z + t * r.direction.z);

            Vec3d outward(axis == 0 ? sign : 0.0,
                          axis == 1 ? sign : 0.0,
                          axis == 2 ? sign : 0.0);

            bool front = (r.direction[axis] * sign) < 0.0;

            HitRecord rec;
            rec.t          = t;
            rec.point      = point;
            rec.front_face = front;
            rec.normal     = front ? outward : -outward;
            rec.material   = material;
            rec.emission   = emission;

            auto uv = uv_at(point, outward);
            rec.u   = uv.first;
            rec.v   = uv.second;

            return rec;
        }

        std::optional<AABB> bounding_box() const override {
            return AABB(pmin, pmax);
        }

        std::pair<double, double> uv_at(const Vec3d& p, const Vec3d& normal) const {
            Vec3d size(pmax.x - pmin.x, pmax.y - pmin.y, pmax.z - pmin.z);
            double u = 0.0, v = 0.0;

            if (std::abs(normal.x) > 0.5) {
                u = (p.z - pmin.z) / size.z;
                v = (p.y - pmin.y) / size.y;
            } else if (std::abs(normal.y) > 0.5) {
                u = (p.x - pmin.x) / size.x;
                v = (p.z - pmin.z) / size.z;
            } else {
                u = (p.x - pmin.x) / size.x;
                v = (p.y - pmin.y) / size.y;
            }
            return {u, v};
        }
    };

    template<> struct shape_traits<Box> {
        static constexpr bool is_closed = true;
        static constexpr bool is_convex = true;
    };
}
