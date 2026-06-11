#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <memory>
#include "core/vec3.h"
#include "geometry/aabb.h"
#include "geometry/bvh.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "geometry/mesh.h"

using namespace rt;
using Catch::Approx;

static Material dummy_mat = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return Scatter{Vec3d(1, 1, 1), Ray(Vec3d(0, 0, 0), Vec3d(0, 0, -1))};
};

// ═══════════════════════════════════════════════════════════════════════════════
// Vec3::operator[]
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Vec3 / operator[] retourne x y z par indice") {
    Vec3d v(1.0, 2.0, 3.0);
    REQUIRE(v[0] == Approx(1.0));
    REQUIRE(v[1] == Approx(2.0));
    REQUIRE(v[2] == Approx(3.0));
}

// ═══════════════════════════════════════════════════════════════════════════════
// AABB::hit() — slab method
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("AABB / rayon traversant la boîte de face") {
    AABB box(Vec3d(-1, -1, -1), Vec3d(1, 1, 1));
    Ray r(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    REQUIRE(box.hit(r, 0.001, 1000.0));
}

TEST_CASE("AABB / rayon passant complètement à côté") {
    AABB box(Vec3d(-1, -1, -1), Vec3d(1, 1, 1));
    Ray r(Vec3d(3, 0, 5), Vec3d(0, 0, -1));  // x=3, hors de [-1,1]
    REQUIRE_FALSE(box.hit(r, 0.001, 1000.0));
}

TEST_CASE("AABB / rayon depuis l'intérieur de la boîte") {
    AABB box(Vec3d(-1, -1, -1), Vec3d(1, 1, 1));
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    REQUIRE(box.hit(r, 0.001, 1000.0));
}

TEST_CASE("AABB / rayon parallèle à une face en dehors") {
    AABB box(Vec3d(-1, -1, -1), Vec3d(1, 1, 1));
    // y=2 : rayon parallèle à l'axe z mais au-dessus de la boîte
    Ray r(Vec3d(0, 2, 0), Vec3d(0, 0, -1));
    REQUIRE_FALSE(box.hit(r, 0.001, 1000.0));
}

TEST_CASE("AABB / rayon diagonal traversant la boîte") {
    AABB box(Vec3d(0, 0, 0), Vec3d(2, 2, 2));
    Vec3d dir = Vec3d(1, 1, 1).normalized();
    Ray r(Vec3d(-1, -1, -1), dir);
    REQUIRE(box.hit(r, 0.001, 1000.0));
}

TEST_CASE("AABB / rayon rejeté par tmax trop petit") {
    AABB box(Vec3d(-1, -1, -1), Vec3d(1, 1, 1));
    Ray r(Vec3d(0, 0, 10), Vec3d(0, 0, -1));  // entre à t≈9
    REQUIRE_FALSE(box.hit(r, 0.001, 5.0));
}

// ═══════════════════════════════════════════════════════════════════════════════
// surrounding_box
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("surrounding_box / union de deux boîtes disjointes") {
    AABB a(Vec3d(0, 0, 0), Vec3d(1, 1, 1));
    AABB b(Vec3d(2, 2, 2), Vec3d(3, 3, 3));
    AABB result = surrounding_box(a, b);
    REQUIRE(result.min_pt.x == Approx(0.0));
    REQUIRE(result.min_pt.y == Approx(0.0));
    REQUIRE(result.min_pt.z == Approx(0.0));
    REQUIRE(result.max_pt.x == Approx(3.0));
    REQUIRE(result.max_pt.y == Approx(3.0));
    REQUIRE(result.max_pt.z == Approx(3.0));
}

TEST_CASE("surrounding_box / une boîte contenue dans l'autre") {
    AABB outer(Vec3d(-5, -5, -5), Vec3d(5, 5, 5));
    AABB inner(Vec3d(-1, -1, -1), Vec3d(1, 1, 1));
    AABB result = surrounding_box(outer, inner);
    REQUIRE(result.min_pt.x == Approx(-5.0));
    REQUIRE(result.max_pt.x == Approx(5.0));
}

TEST_CASE("surrounding_box / boîtes se chevauchant partiellement") {
    AABB a(Vec3d(0, 0, 0), Vec3d(2, 2, 2));
    AABB b(Vec3d(1, 1, 1), Vec3d(4, 4, 4));
    AABB result = surrounding_box(a, b);
    REQUIRE(result.min_pt.x == Approx(0.0));
    REQUIRE(result.max_pt.x == Approx(4.0));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Sphere::bounding_box()
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Sphere / bounding_box a une valeur") {
    Sphere s(Vec3d(0, 0, 0), 1.0, dummy_mat);
    REQUIRE(s.bounding_box().has_value());
}

TEST_CASE("Sphere / bounding_box correcte center=(1,2,3) radius=0.5") {
    Sphere s(Vec3d(1, 2, 3), 0.5, dummy_mat);
    auto box = s.bounding_box();
    REQUIRE(box.has_value());
    REQUIRE(box->min_pt.x == Approx(0.5));
    REQUIRE(box->min_pt.y == Approx(1.5));
    REQUIRE(box->min_pt.z == Approx(2.5));
    REQUIRE(box->max_pt.x == Approx(1.5));
    REQUIRE(box->max_pt.y == Approx(2.5));
    REQUIRE(box->max_pt.z == Approx(3.5));
}

TEST_CASE("Sphere / la boîte contient bien la sphère — rayon doit la traverser") {
    Sphere s(Vec3d(0, 0, -3), 1.0, dummy_mat);
    auto box = s.bounding_box();
    REQUIRE(box.has_value());
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    REQUIRE(box->hit(r, 0.001, 1000.0));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Triangle::bounding_box()
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Triangle / bounding_box a une valeur") {
    Triangle tri(Vec3d(0, 0, -2), Vec3d(1, 0, -2), Vec3d(0, 1, -2), dummy_mat);
    REQUIRE(tri.bounding_box().has_value());
}

TEST_CASE("Triangle / bounding_box contient les 3 sommets") {
    Vec3d A(0, 0, -2), B(3, 0, -2), C(1, 2, -2);
    Triangle tri(A, B, C, dummy_mat);
    auto box = tri.bounding_box();
    REQUIRE(box.has_value());
    REQUIRE(box->min_pt.x <= 0.0);
    REQUIRE(box->min_pt.y <= 0.0);
    REQUIRE(box->max_pt.x >= 3.0);
    REQUIRE(box->max_pt.y >= 2.0);
}

TEST_CASE("Triangle / bounding_box non dégénérée sur un triangle plat (epsilon)") {
    // Triangle dans le plan z=-2 : sans epsilon, min_z == max_z
    Vec3d A(0, 0, -2), B(1, 0, -2), C(0, 1, -2);
    Triangle tri(A, B, C, dummy_mat);
    auto box = tri.bounding_box();
    REQUIRE(box.has_value());
    REQUIRE(box->max_pt.z - box->min_pt.z > 0.0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Mesh::bounding_box()
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Mesh vide / bounding_box retourne nullopt") {
    Mesh mesh(dummy_mat);
    REQUIRE_FALSE(mesh.bounding_box().has_value());
}

TEST_CASE("Mesh / bounding_box est l'union des triangles") {
    Mesh mesh(dummy_mat);
    mesh.add(Triangle(Vec3d(0, 0, 0), Vec3d(1, 0, 0), Vec3d(0, 1, 0), dummy_mat));
    mesh.add(Triangle(Vec3d(5, 5, 5), Vec3d(6, 5, 5), Vec3d(5, 6, 5), dummy_mat));
    auto box = mesh.bounding_box();
    REQUIRE(box.has_value());
    REQUIRE(box->min_pt.x <= 0.0);
    REQUIRE(box->max_pt.x >= 6.0);
    REQUIRE(box->max_pt.y >= 6.0);
    REQUIRE(box->max_pt.z >= 5.0);
}

TEST_CASE("Mesh / bounding_box d'un seul triangle == bounding_box du triangle") {
    Triangle tri(Vec3d(0, 0, -2), Vec3d(1, 0, -2), Vec3d(0, 1, -2), dummy_mat);
    Mesh mesh(dummy_mat);
    mesh.add(tri);
    auto mesh_box = mesh.bounding_box();
    auto tri_box  = tri.bounding_box();
    REQUIRE(mesh_box.has_value());
    REQUIRE(mesh_box->min_pt.x == Approx(tri_box->min_pt.x));
    REQUIRE(mesh_box->max_pt.x == Approx(tri_box->max_pt.x));
}

// ═══════════════════════════════════════════════════════════════════════════════
// BVHNode::hit()
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("BVH / rayon frappe la sphère la plus proche (2 sphères)") {
    auto s1 = std::make_shared<Sphere>(Vec3d(0, 0, -2), 0.5, dummy_mat);
    auto s2 = std::make_shared<Sphere>(Vec3d(0, 0, -5), 0.5, dummy_mat);
    std::vector<std::shared_ptr<Shape>> shapes = {s1, s2};
    BVHPool pool(2); BVHNode* bvh = pool.build(shapes, 0, 2);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    auto hit = bvh->hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->t == Approx(1.5).epsilon(0.01));
}

TEST_CASE("BVH / rayon manque toutes les shapes") {
    auto s1 = std::make_shared<Sphere>(Vec3d(0, 0, -2), 0.5, dummy_mat);
    auto s2 = std::make_shared<Sphere>(Vec3d(0, 0, -5), 0.5, dummy_mat);
    std::vector<std::shared_ptr<Shape>> shapes = {s1, s2};
    BVHPool pool(2); BVHNode* bvh = pool.build(shapes, 0, 2);
    Ray r(Vec3d(5, 5, 0), Vec3d(0, 0, -1));
    REQUIRE_FALSE(bvh->hit(r, 0.001, 1000.0).has_value());
}

TEST_CASE("BVH / 3 sphères — retourne la plus proche") {
    auto s1 = std::make_shared<Sphere>(Vec3d(0, 0, -2), 0.5, dummy_mat);
    auto s2 = std::make_shared<Sphere>(Vec3d(0, 0, -5), 0.5, dummy_mat);
    auto s3 = std::make_shared<Sphere>(Vec3d(0, 0, -8), 0.5, dummy_mat);
    std::vector<std::shared_ptr<Shape>> shapes = {s1, s2, s3};
    BVHPool pool(3); BVHNode* bvh = pool.build(shapes, 0, 3);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    auto hit = bvh->hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->t == Approx(1.5).epsilon(0.01));
}

TEST_CASE("BVH / rayon rejeté par tmax avant toutes les sphères") {
    auto s1 = std::make_shared<Sphere>(Vec3d(0, 0, -5), 0.5, dummy_mat);
    auto s2 = std::make_shared<Sphere>(Vec3d(0, 0, -8), 0.5, dummy_mat);
    std::vector<std::shared_ptr<Shape>> shapes = {s1, s2};
    BVHPool pool(2); BVHNode* bvh = pool.build(shapes, 0, 2);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    REQUIRE_FALSE(bvh->hit(r, 0.001, 2.0).has_value());  // tmax=2, sphères à t≈4.5
}

TEST_CASE("BVH / 1 seule shape") {
    auto s1 = std::make_shared<Sphere>(Vec3d(0, 0, -3), 0.5, dummy_mat);
    std::vector<std::shared_ptr<Shape>> shapes = {s1};
    BVHPool pool(1); BVHNode* bvh = pool.build(shapes, 0, 1);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    auto hit = bvh->hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->t == Approx(2.5).epsilon(0.01));
}
