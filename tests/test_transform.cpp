#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <memory>
#include <numbers>
#include "core/mat4.h"
#include "geometry/sphere.h"
#include "scene/transform_node.h"

using namespace rt;
using Catch::Approx;

static const double PI = std::numbers::pi;

static Material dummy_mat = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return Scatter{Vec3d(1,1,1), Ray(Vec3d(0,0,0), Vec3d(0,0,-1))};
};

// ═══════════════════════════════════════════════════════════════════════════════
// Mat4 — opérations de base
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Mat4 / identity * point == point", "[transform]") {
    Mat4 I = Mat4::identity();
    Vec3d p(1, 2, 3);
    Vec3d result = I.transform_point(p);
    REQUIRE(result.x == Approx(1.0));
    REQUIRE(result.y == Approx(2.0));
    REQUIRE(result.z == Approx(3.0));
}

TEST_CASE("Mat4 / translate déplace un point", "[transform]") {
    Mat4 T = Mat4::translate(1, 0, 0);
    Vec3d p(0, 0, 0);
    Vec3d result = T.transform_point(p);
    REQUIRE(result.x == Approx(1.0));
    REQUIRE(result.y == Approx(0.0));
    REQUIRE(result.z == Approx(0.0));
}

TEST_CASE("Mat4 / translate ne déplace pas une direction", "[transform]") {
    Mat4 T = Mat4::translate(5, 5, 5);
    Vec3d d(1, 0, 0);
    Vec3d result = T.transform_dir(d);
    REQUIRE(result.x == Approx(1.0));
    REQUIRE(result.y == Approx(0.0));
    REQUIRE(result.z == Approx(0.0));
}

TEST_CASE("Mat4 / scale multiplie les coordonnées", "[transform]") {
    Mat4 S = Mat4::scale(2, 2, 2);
    Vec3d p(1, 1, 1);
    Vec3d result = S.transform_point(p);
    REQUIRE(result.x == Approx(2.0));
    REQUIRE(result.y == Approx(2.0));
    REQUIRE(result.z == Approx(2.0));
}

TEST_CASE("Mat4 / rotate_y(90°) tourne (1,0,0) vers (0,0,-1)", "[transform]") {
    Mat4 R = Mat4::rotate_y(PI / 2.0);
    Vec3d d(1, 0, 0);
    Vec3d result = R.transform_dir(d);
    REQUIRE(result.x == Approx(0.0).margin(1e-9));
    REQUIRE(result.y == Approx(0.0).margin(1e-9));
    REQUIRE(result.z == Approx(-1.0).epsilon(0.001));
}

TEST_CASE("Mat4 / inverse annule la translation", "[transform]") {
    Mat4 T = Mat4::translate(3, -1, 2);
    Mat4 Tinv = T.inverse();
    Vec3d p(3, -1, 2);
    Vec3d result = Tinv.transform_point(p);
    REQUIRE(result.x == Approx(0.0).margin(1e-9));
    REQUIRE(result.y == Approx(0.0).margin(1e-9));
    REQUIRE(result.z == Approx(0.0).margin(1e-9));
}

TEST_CASE("Mat4 / M * M_inverse == identity", "[transform]") {
    Mat4 T = Mat4::translate(1, 2, 3);
    Mat4 S = Mat4::scale(2, 3, 0.5);
    Mat4 M = T * S;
    Mat4 Minv = M.inverse();
    Vec3d p(5, -2, 1);
    Vec3d roundtrip = Minv.transform_point(M.transform_point(p));
    REQUIRE(roundtrip.x == Approx(p.x).margin(1e-9));
    REQUIRE(roundtrip.y == Approx(p.y).margin(1e-9));
    REQUIRE(roundtrip.z == Approx(p.z).margin(1e-9));
}

// ═══════════════════════════════════════════════════════════════════════════════
// TransformNode — position et hit
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("TransformNode / sphère translatee frappe au bon endroit", "[transform]") {
    auto sphere = std::make_shared<Sphere>(Vec3d(0,0,0), 1.0, dummy_mat);
    auto moved  = make_translate(sphere, 0, 2, 0);   // sphère à (0,2,0)

    // rayon tiré depuis y=-10 vers y=+1 (devrait frapper la sphère à y≈1)
    Ray r(Vec3d(0, -10, 0), Vec3d(0, 1, 0));
    auto hit = moved->hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->point.y == Approx(1.0).epsilon(0.01));
}

TEST_CASE("TransformNode / sphère translatee ne frappe pas à l'origine", "[transform]") {
    auto sphere = std::make_shared<Sphere>(Vec3d(0,0,0), 0.5, dummy_mat);
    auto moved  = make_translate(sphere, 10, 0, 0);  // déplacée loin

    // rayon passant par l'origine (là où était la sphère)
    Ray r(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    auto hit = moved->hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("TransformNode / normale correcte après scale non-uniforme", "[transform]") {
    // Scale x2 sur Y : une sphère unité devient un ellipsoïde aplati en Y
    auto sphere = std::make_shared<Sphere>(Vec3d(0,0,0), 1.0, dummy_mat);
    auto scaled = make_scale(sphere, 1.0, 2.0, 1.0);

    // rayon depuis le haut visant le pôle nord de l'ellipsoïde
    Ray r(Vec3d(0, 10, 0), Vec3d(0, -1, 0));
    auto hit = scaled->hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    // la normale au pôle doit pointer vers +Y
    REQUIRE(hit->normal.y == Approx(1.0).epsilon(0.01));
    REQUIRE(hit->normal.x == Approx(0.0).margin(0.01));
}

TEST_CASE("TransformNode / AABB englobe la sphère transformée", "[transform]") {
    auto sphere = std::make_shared<Sphere>(Vec3d(0,0,0), 1.0, dummy_mat);
    auto moved  = make_translate(sphere, 5, 0, 0);

    auto bb = moved->bounding_box();
    REQUIRE(bb.has_value());
    REQUIRE(bb->min_pt.x == Approx(4.0).epsilon(0.01));
    REQUIRE(bb->max_pt.x == Approx(6.0).epsilon(0.01));
}

// ═══════════════════════════════════════════════════════════════════════════════
// CRTP Transformable
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Transformable / sphere->translate compile et retourne un Shape", "[transform]") {
    auto sphere = std::make_shared<Sphere>(Vec3d(0,0,0), 1.0, dummy_mat);
    auto moved  = sphere->translate(1, 0, 0);

    REQUIRE(moved != nullptr);
    Ray r(Vec3d(-10, 0, 0), Vec3d(1, 0, 0));
    auto hit = moved->hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
}
