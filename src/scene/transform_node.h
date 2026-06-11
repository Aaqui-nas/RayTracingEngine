#pragma once
#include "core/mat4.h"
#include "core/vec3.h"
#include "geometry/shape.h"
#include "core/ray.h"
#include <memory>
#include <optional>
#include <variant>

namespace rt{
    class TransformNode : public Shape {
        std::shared_ptr<Shape> child;
        Mat4 world_to_local;
        Mat4 local_to_world;
        Mat4 normal_matrix;

    public:
        TransformNode(std::shared_ptr<Shape> child, const Mat4& transform) : Shape(Material{}), child(child), local_to_world(transform) {
            world_to_local = local_to_world.inverse();
            normal_matrix = world_to_local.transpose();
        }

        std::optional<HitRecord> hit(const Ray& r, double tmin, double tmax) const override {
            Vec3d origin_local = world_to_local.transform_point(r.origin);
            Vec3d direction_local = world_to_local.transform_dir(r.direction);
            Ray ray_local = Ray(origin_local, direction_local);

            auto rec = child->hit(ray_local, tmin, tmax);
            if (!rec) return std::nullopt;

            rec->point = local_to_world.transform_point(rec->point);
            rec->normal = normal_matrix.transform_dir(rec->normal).normalized();
            rec->front_face = dot(r.direction, rec->normal) < 0;
            return rec;
        }
        std::optional<AABB> bounding_box() const override {
            std::optional<AABB> aabb = child->bounding_box();
            if(!aabb) return std::nullopt;
            Vec3d points[2] = {aabb->min_pt, aabb->max_pt};
            Vec3d coin0 = local_to_world.transform_point(Vec3d(points[0].x,points[0].y,points[0].z));
            double minx = coin0.x; double miny = coin0.y; double minz = coin0.z;
            double maxx= coin0.x; double maxy = coin0.y; double maxz = coin0.z;
            for (int ix : {0,1}) {
                for (int iy : {0,1}) {
                    for (int iz : {0,1}) {
                        Vec3d coin(points[ix].x,points[iy].y,points[iz].z);
                        Vec3d coin_t = local_to_world.transform_point(coin);
                        minx = std::min(minx, coin_t.x);
                        maxx = std::max(maxx, coin_t.x);
                        miny = std::min(miny, coin_t.y);
                        maxy = std::max(maxy, coin_t.y);
                        minz = std::min(minz, coin_t.z);
                        maxz = std::max(maxz, coin_t.z);
                    }
                }
            }
            return AABB(Vec3d(minx, miny, minz), Vec3d(maxx, maxy, maxz));
        }
    };

    inline std::shared_ptr<Shape> make_translate(
        std::shared_ptr<Shape> shape, double dx, double dy, double dz) {
        return std::make_shared<TransformNode>(shape, Mat4::translate(dx,dy,dz));
    }

    inline std::shared_ptr<Shape> make_rotate_x(
        std::shared_ptr<Shape> shape, double angle_degrees) {
        double angle = angle_degrees * pi / 180.0;
        return std::make_shared<TransformNode>(shape, Mat4::rotate_x(angle));
    }
    inline std::shared_ptr<Shape> make_rotate_y(
        std::shared_ptr<Shape> shape, double angle_degrees) {
        double angle = angle_degrees * pi / 180.0;
        return std::make_shared<TransformNode>(shape, Mat4::rotate_y(angle));
    }
    inline std::shared_ptr<Shape> make_rotate_z(
        std::shared_ptr<Shape> shape, double angle_degrees) {
        double angle = angle_degrees * pi / 180.0;
        return std::make_shared<TransformNode>(shape, Mat4::rotate_z(angle));
    }

    inline std::shared_ptr<Shape> make_scale(
        std::shared_ptr<Shape> shape, double sx, double sy, double sz) {
        return std::make_shared<TransformNode>(shape, Mat4::scale(sx, sy, sz));
    }

    struct SceneNode;
    struct LeafNode {std::shared_ptr<Shape> shape;};
    struct GroupNode{ Mat4 transform; std::vector<std::shared_ptr<SceneNode>> children;};
    struct SceneNode : std::variant<LeafNode, GroupNode> {
        using variant::variant;
    };

    struct BoundVisitor {
        std::optional<AABB> operator()(const LeafNode& leaf) {
            return leaf.shape->bounding_box();
        }
        std::optional<AABB> operator()(const GroupNode& group) {
            std::vector<std::optional<AABB>> aabbs;
            for (int i = 0; i < group.children.size(); i++) {
                std::optional<AABB> aabb = std::visit(BoundVisitor{},*group.children[i]);
                if(aabb) {
                    Vec3d points[2] = {aabb->min_pt, aabb->max_pt};
                    Vec3d coin0 = group.transform.transform_point(Vec3d(points[0].x,points[0].y,points[0].z));
                    double minx = coin0.x; double miny = coin0.y; double minz = coin0.z;
                    double maxx= coin0.x; double maxy = coin0.y; double maxz = coin0.z;
                    for (int ix : {0,1}) {
                        for (int iy : {0,1}) {
                            for (int iz : {0,1}) {
                                Vec3d coin(points[ix].x,points[iy].y,points[iz].z);
                                Vec3d coin_t = group.transform.transform_point(coin);
                                minx = std::min(minx, coin_t.x);
                                maxx = std::max(maxx, coin_t.x);
                                miny = std::min(miny, coin_t.y);
                                maxy = std::max(maxy, coin_t.y);
                                minz = std::min(minz, coin_t.z);
                                maxz = std::max(maxz, coin_t.z);
                            }
                        }
                    }
                    aabbs.push_back(AABB(Vec3d(minx, miny, minz), Vec3d(maxx, maxy, maxz)));
                }
            }
            if (aabbs.empty()) return std::nullopt;
            std::optional<AABB> result = aabbs[0];
            for (int i = 1; i < aabbs.size(); i++) {
                result = surrounding_box(result.value(), aabbs[i].value());
            }
            return result;
        }
    };

    template<typename Derived>
    class Transformable {
    public:
        std::shared_ptr<Shape> translate(double dx, double dy, double dz) const {
            return make_translate(
                std::static_pointer_cast<Shape>(
                    std::make_shared<Derived>(static_cast<const Derived&>(*this))),
                dx, dy, dz);
        }

        std::shared_ptr<Shape> rotate_x(double degrees) const {
            return make_rotate_x(
                std::static_pointer_cast<Shape>(
                    std::make_shared<Derived>(static_cast<const Derived&>(*this)))
                , degrees);
        }

        std::shared_ptr<Shape> rotate_y(double degrees) const {
            return make_rotate_y(
                std::static_pointer_cast<Shape>(
                    std::make_shared<Derived>(static_cast<const Derived&>(*this)))
                , degrees);
        }

        std::shared_ptr<Shape> rotate_z(double degrees) const {
            return make_rotate_z(
                std::static_pointer_cast<Shape>(
                    std::make_shared<Derived>(static_cast<const Derived&>(*this)))
                , degrees);
        }
        std::shared_ptr<Shape> scale(double s) const {
            return make_scale(
                std::static_pointer_cast<Shape>(
                    std::make_shared<Derived>(static_cast<const Derived&>(*this)))
                , s, s, s);
        }
    };
}
