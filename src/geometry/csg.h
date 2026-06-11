#pragma once
#include <optional>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>

#include "geometry/shape.h"

namespace rt {

    enum class CsgOp { Union, Intersection, Difference };

    template<CsgOp Op>
    class CsgNode : public Shape {
        std::shared_ptr<Shape> left, right;

        static bool inside(bool a, bool b) {
            if constexpr (Op == CsgOp::Union)             return a || b;
            else if constexpr (Op == CsgOp::Intersection) return a && b;
            else                                          return a && !b;
        }

        struct Boundary {
            double    t;
            bool      enter;
            int       shape;
            HitRecord rec;
        };

        static void collect(const Shape& s, const Ray& r, int which,
                            std::vector<Boundary>& out) {
            constexpr double LO = -1e30, HI = 1e30;
            double t = LO;
            for (int guard = 0; guard < 64; ++guard) {
                auto h = s.hit(r, t, HI);
                if (!h) break;
                HitRecord rec = *h;
                bool enter = rec.front_face;
                Vec3d outward = rec.front_face ? rec.normal : -rec.normal;
                rec.normal = outward;
                out.push_back({rec.t, enter, which, rec});
                t = rec.t + 1e-6;
            }
        }

    public:
        CsgNode(std::shared_ptr<Shape> a, std::shared_ptr<Shape> b, Material mat = {})
            : Shape(std::move(mat)), left(std::move(a)), right(std::move(b)) {}

        std::optional<HitRecord> hit(const Ray& r, double tmin, double tmax) const override {
            std::vector<Boundary> ev;
            collect(*left,  r, 0, ev);
            collect(*right, r, 1, ev);
            if (ev.empty()) return std::nullopt;

            std::sort(ev.begin(), ev.end(), [](const Boundary& x, const Boundary& y) {
                if (x.t != y.t) return x.t < y.t;
                return x.enter && !y.enter;
            });

            bool inL = false, inR = false;
            bool combined = inside(false, false);

            for (const auto& e : ev) {
                if (e.shape == 0) inL = e.enter; else inR = e.enter;
                bool now = inside(inL, inR);
                if (now == combined) continue;
                combined = now;

                if (e.t < tmin || e.t > tmax) continue;

                HitRecord rec = e.rec;
                if constexpr (Op == CsgOp::Difference) {
                    if (e.shape == 1)
                        rec.normal = -rec.normal;
                }

                double dn = r.direction.x * rec.normal.x
                          + r.direction.y * rec.normal.y
                          + r.direction.z * rec.normal.z;
                bool front = dn < 0.0;
                rec.front_face = front;
                if (!front)
                    rec.normal = -rec.normal;

                return rec;
            }
            return std::nullopt;
        }

        std::optional<AABB> bounding_box() const override {
            auto la = left->bounding_box();
            auto rb = right->bounding_box();

            if constexpr (Op == CsgOp::Difference) {
                return la;
            } else if constexpr (Op == CsgOp::Intersection) {
                if (la && rb)
                    return AABB(
                        Vec3d(std::max(la->min_pt.x, rb->min_pt.x),
                              std::max(la->min_pt.y, rb->min_pt.y),
                              std::max(la->min_pt.z, rb->min_pt.z)),
                        Vec3d(std::min(la->max_pt.x, rb->max_pt.x),
                              std::min(la->max_pt.y, rb->max_pt.y),
                              std::min(la->max_pt.z, rb->max_pt.z)));
                return la ? la : rb;
            } else {
                if (la && rb) return surrounding_box(*la, *rb);
                return la ? la : rb;
            }
        }
    };

    inline std::shared_ptr<Shape> csg_union(std::shared_ptr<Shape> a, std::shared_ptr<Shape> b) {
        return std::make_shared<CsgNode<CsgOp::Union>>(std::move(a), std::move(b));
    }

    inline std::shared_ptr<Shape> csg_inter(std::shared_ptr<Shape> a, std::shared_ptr<Shape> b) {
        return std::make_shared<CsgNode<CsgOp::Intersection>>(std::move(a), std::move(b));
    }

    inline std::shared_ptr<Shape> csg_diff(std::shared_ptr<Shape> a, std::shared_ptr<Shape> b) {
        return std::make_shared<CsgNode<CsgOp::Difference>>(std::move(a), std::move(b));
    }
}
