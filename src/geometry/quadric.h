#pragma once
#include <optional>
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>

#include "geometry/shape.h"
#include "geometry/shape_traits.h"

namespace rt {

namespace detail {


    template<bool IsCone>
    std::optional<HitRecord> intersect_quadric(
        const Ray& r, double tmin, double tmax,
        double radius, double height, double half_angle = 0.0)
    {
        const Vec3d& o = r.origin;
        const Vec3d& d = r.direction;

        const double dtan  = std::tan(half_angle);
        const double dtan2 = dtan * dtan;

        double a, b, c;
        if constexpr (IsCone) {
            a = d.x*d.x + d.z*d.z - dtan2*d.y*d.y;
            b = 2.0*(o.x*d.x + o.z*d.z - dtan2*o.y*d.y);
            c = o.x*o.x + o.z*o.z - dtan2*o.y*o.y;
        } else {
            a = d.x*d.x + d.z*d.z;
            b = 2.0*(o.x*d.x + o.z*d.z);
            c = o.x*o.x + o.z*o.z - radius*radius;
        }

        auto normal_at = [&](const Vec3d& p) -> Vec3d {
            if constexpr (IsCone) return compute_normal(p, half_angle, height, ConeTag{});
            else                  return compute_normal(p, radius, height, CylinderTag{});
        };

        std::optional<HitRecord> best;
        double closest = tmax;

        auto commit = [&](double t, const Vec3d& p) {
            if (t < tmin || t > closest) return;
            Vec3d outward = normal_at(p);
            double len = std::sqrt(outward.x*outward.x + outward.y*outward.y + outward.z*outward.z);
            Vec3d n = (len > 1e-12)
                ? Vec3d(outward.x/len, outward.y/len, outward.z/len)
                : Vec3d(0.0, 1.0, 0.0);
            double dn = d.x*n.x + d.y*n.y + d.z*n.z;
            bool front = dn < 0.0;
            HitRecord rec;
            rec.t          = t;
            rec.point      = p;
            rec.front_face = front;
            rec.normal     = front ? n : -n;
            closest = t;
            best = rec;
        };

        auto body = [&](double t) {
            if (t < tmin || t > closest) return;
            double y = o.y + t*d.y;
            if (y < 0.0 || y > height) return;
            commit(t, Vec3d(o.x + t*d.x, y, o.z + t*d.z));
        };

        if (std::abs(a) > 1e-12) {
            double disc = b*b - 4.0*a*c;
            if (disc >= 0.0) {
                double sq = std::sqrt(disc);
                double t0 = (-b - sq) / (2.0*a);
                double t1 = (-b + sq) / (2.0*a);
                if (t0 > t1) std::swap(t0, t1);
                body(t0);
                body(t1);
            }
        } else if (std::abs(b) > 1e-12) {
            body(-c / b);
        }

        if (std::abs(d.y) > 1e-12) {
            if constexpr (IsCone) {
                double t = (height - o.y) / d.y;
                Vec3d p(o.x + t*d.x, height, o.z + t*d.z);
                if (p.x*p.x + p.z*p.z <= radius*radius) commit(t, p);
            } else {
                for (double cap_y : {0.0, height}) {
                    double t = (cap_y - o.y) / d.y;
                    Vec3d p(o.x + t*d.x, cap_y, o.z + t*d.z);
                    if (p.x*p.x + p.z*p.z <= radius*radius) commit(t, p);
                }
            }
        }

        if (best) {
            HitRecord& rec = *best;
            rec.v = height > 0.0 ? rec.point.y / height : 0.0;
            rec.u = (std::atan2(rec.point.z, rec.point.x) + pi) / (2.0 * pi);
        }

        return best;
    }

}

    class Cylinder : public Shape {
        double radius;
        double height;
    public:
        Cylinder(double radius, double height, Material mat)
            : Shape(std::move(mat)), radius(radius), height(height) {}

        std::optional<HitRecord> hit(const Ray& r, double tmin, double tmax) const override {
            auto rec = detail::intersect_quadric<false>(r, tmin, tmax, radius, height);
            if (rec) {
                rec->material = material;
                rec->emission = emission;
            }
            return rec;
        }

        std::optional<AABB> bounding_box() const override {
            return AABB(Vec3d(- radius, 0, - radius),
                        Vec3d(radius, height, radius));
        }
    };

    class Cone : public Shape {
        double base_radius;
        double height;
        double half_angle;
    public:
        Cone(double base_radius, double height, Material mat)
            : Shape(std::move(mat)), base_radius(base_radius),
              height(height), half_angle(std::atan2(base_radius, height)) {}

        std::optional<HitRecord> hit(const Ray& r, double tmin, double tmax) const override {
            auto rec = detail::intersect_quadric<true>(r, tmin, tmax, base_radius, height, half_angle);
            if (rec) {
                rec->material = material;
                rec->emission = emission;
            }
            return rec;
        }

        std::optional<AABB> bounding_box() const override {
            return AABB(Vec3d(-base_radius, 0, -base_radius),
                        Vec3d(base_radius, height, base_radius));
        }
    };

    template<> struct shape_traits<Cylinder> {
        static constexpr bool is_closed = true;
        static constexpr bool is_convex = true;
    };

    template<> struct shape_traits<Cone> {
        static constexpr bool is_closed = true;
        static constexpr bool is_convex = true;
    };

}
