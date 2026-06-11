#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <memory>
#include <numbers>
#include "geometry/sphere.h"
#include "geometry/box.h"
#include "geometry/quadric.h"
#include "geometry/csg.h"
#include "geometry/shape_traits.h"

using namespace rt;
using Catch::Approx;

static Material dummy = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return Scatter{Vec3d(1,1,1), Ray(Vec3d(0,0,0), Vec3d(0,0,-1))};
};

// ═══════════════════════════════════════════════════════════════════════════════
// Box
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Box / rayon depuis +Z frappe la face avant", "[shapes]") {
    Box box(Vec3d(-1,-1,-1), Vec3d(1,1,1), dummy);
    Ray r(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    auto hit = box.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->t        == Approx(4.0).epsilon(0.01));
    REQUIRE(hit->normal.z == Approx(1.0).epsilon(0.01));
    REQUIRE(hit->front_face == true);
}

TEST_CASE("Box / rayon depuis +X frappe la face +X", "[shapes]") {
    Box box(Vec3d(-1,-1,-1), Vec3d(1,1,1), dummy);
    Ray r(Vec3d(5, 0, 0), Vec3d(-1, 0, 0));
    auto hit = box.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->normal.x == Approx(1.0).epsilon(0.01));
}

TEST_CASE("Box / rayon depuis +Y frappe la face +Y", "[shapes]") {
    Box box(Vec3d(-1,-1,-1), Vec3d(1,1,1), dummy);
    Ray r(Vec3d(0, 5, 0), Vec3d(0, -1, 0));
    auto hit = box.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->normal.y == Approx(1.0).epsilon(0.01));
}

TEST_CASE("Box / rayon passant à côté ne frappe pas", "[shapes]") {
    Box box(Vec3d(-1,-1,-1), Vec3d(1,1,1), dummy);
    Ray r(Vec3d(2, 0, 5), Vec3d(0, 0, -1));
    auto hit = box.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Box / bounding_box retourne les coins exacts", "[shapes]") {
    Box box(Vec3d(-2, -3, -1), Vec3d(4, 1, 2), dummy);
    auto bb = box.bounding_box();

    REQUIRE(bb.has_value());
    REQUIRE(bb->min_pt.x == Approx(-2.0));
    REQUIRE(bb->max_pt.y == Approx(1.0));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Cylinder
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Cylinder / rayon depuis +X frappe le corps", "[shapes]") {
    Cylinder cyl(1.0, 2.0, dummy);  // rayon=1, hauteur=2
    Ray r(Vec3d(5, 1, 0), Vec3d(-1, 0, 0));  // à mi-hauteur
    auto hit = cyl.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->normal.x == Approx(1.0).epsilon(0.01));
    REQUIRE(hit->normal.y == Approx(0.0).margin(0.01));
}

TEST_CASE("Cylinder / rayon depuis le dessus frappe le cap supérieur", "[shapes]") {
    Cylinder cyl(1.0, 2.0, dummy);
    Ray r(Vec3d(0, 5, 0), Vec3d(0, -1, 0));
    auto hit = cyl.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->normal.y == Approx(1.0).epsilon(0.01));
    REQUIRE(hit->t        == Approx(3.0).epsilon(0.01));
}

TEST_CASE("Cylinder / rayon depuis le dessous frappe le cap inférieur", "[shapes]") {
    Cylinder cyl(1.0, 2.0, dummy);
    Ray r(Vec3d(0, -5, 0), Vec3d(0, 1, 0));
    auto hit = cyl.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->normal.y == Approx(-1.0).epsilon(0.01));
}

TEST_CASE("Cylinder / rayon passant à côté ne frappe pas", "[shapes]") {
    Cylinder cyl(1.0, 2.0, dummy);
    Ray r(Vec3d(2, 5, 0), Vec3d(0, -1, 0));  // à x=2 > rayon=1
    auto hit = cyl.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Cone
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Cone / rayon depuis +X frappe le corps", "[shapes]") {
    // Cone rayon_base=1, hauteur=2, base en y=0, apex en y=2
    // À y=1 (mi-hauteur), le rayon du cône est 0.5
    Cone cone(1.0, 2.0, dummy);
    Ray r(Vec3d(5, 1, 0), Vec3d(-1, 0, 0));  // vise y=1, à x=5
    auto hit = cone.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->point.y == Approx(1.0).margin(0.01));
}

TEST_CASE("Cone / rayon depuis le dessus frappe le cap de base", "[shapes]") {
    // Rayon depuis y=3 vers le bas — frappe la base (y=height=2) à t=1, avant le corps
    Cone cone(1.0, 2.0, dummy);
    Ray r(Vec3d(0, 3, 0), Vec3d(0, -1, 0));
    auto hit = cone.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->t        == Approx(1.0).epsilon(0.01));
    REQUIRE(hit->normal.y == Approx(1.0).epsilon(0.01));  // outward (0,1,0), front face
}

TEST_CASE("Cone / rayon passant à côté du corps ne frappe pas", "[shapes]") {
    Cone cone(1.0, 2.0, dummy);
    // À y=1 le rayon est 0.5 — un rayon à x=2 doit rater
    Ray r(Vec3d(2, 1, 5), Vec3d(0, 0, -1));
    auto hit = cone.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Type traits (compile-time — vérifiés avec static_assert + tests runtime)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("shape_traits / Sphere est closed et convex", "[shapes]") {
    static_assert(shape_traits<Sphere>::is_closed == true);
    static_assert(shape_traits<Sphere>::is_convex == true);
    SUCCEED();
}

TEST_CASE("shape_traits / Box est closed et convex", "[shapes]") {
    static_assert(shape_traits<Box>::is_closed == true);
    static_assert(shape_traits<Box>::is_convex == true);
    SUCCEED();
}

TEST_CASE("shape_traits / Cone est closed et convex", "[shapes]") {
    static_assert(shape_traits<Cone>::is_closed == true);
    static_assert(shape_traits<Cone>::is_convex == true);
    SUCCEED();
}

TEST_CASE("has_bounding_box / Sphere et Box ont une bounding_box", "[shapes]") {
    static_assert(has_bounding_box<Sphere>::value == true);
    static_assert(has_bounding_box<Box>::value    == true);
    SUCCEED();
}

TEST_CASE("has_bounding_box / int n'a pas de bounding_box", "[shapes]") {
    static_assert(has_bounding_box<int>::value == false);
    SUCCEED();
}

// ═══════════════════════════════════════════════════════════════════════════════
// CSG
// Setup : deux sphères qui se chevauchent partiellement
//   left  = Sphere(0,0,0) r=2   → intervalle t=[3, 7] pour rayon (0,0,5)→(0,0,-1)
//   right = Sphere(1.5,0,0) r=2 → intervalle t≈[3.68, 6.32]
// ═══════════════════════════════════════════════════════════════════════════════

static auto make_csg_left()  { return std::make_shared<Sphere>(Vec3d(0,0,0),   2.0, dummy); }
static auto make_csg_right() { return std::make_shared<Sphere>(Vec3d(1.5,0,0), 2.0, dummy); }
static Ray csg_ray() { return Ray(Vec3d(0, 0, 5), Vec3d(0, 0, -1)); }

TEST_CASE("CSGUnion / frappe si l'une des deux sphères est touchée", "[shapes]") {
    auto u = csg_union(make_csg_left(), make_csg_right());
    auto hit = u->hit(csg_ray(), 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->t == Approx(3.0).epsilon(0.01));  // premier hit : left
}

TEST_CASE("CSGUnion / frappe si seulement right est touchée", "[shapes]") {
    // Rayon décalé en x=1.5 — rate left (centrée en 0), touche right (centrée en 1.5)
    auto u = csg_union(make_csg_left(), make_csg_right());
    Ray r(Vec3d(1.5, 0, 5), Vec3d(0, 0, -1));

    // left (centre 0, r=2) : dist de l'axe = 1.5 < 2 → touchée aussi
    // Testons avec une sphère qui ne se touche vraiment pas
    auto left_far  = std::make_shared<Sphere>(Vec3d(0, 10, 0), 1.0, dummy);
    auto right_near = std::make_shared<Sphere>(Vec3d(0, 0, -3), 1.0, dummy);
    auto u2 = csg_union(left_far, right_near);
    auto hit = u2->hit(csg_ray(), 0.001, 1000.0);

    REQUIRE(hit.has_value());  // frappe right_near
}

TEST_CASE("CSGUnion / ne frappe pas si aucune sphère n'est touchée", "[shapes]") {
    auto a = std::make_shared<Sphere>(Vec3d(10, 0, 0), 1.0, dummy);
    auto b = std::make_shared<Sphere>(Vec3d(-10, 0, 0), 1.0, dummy);
    auto u = csg_union(a, b);
    auto hit = u->hit(csg_ray(), 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("CSGIntersection / frappe dans la zone de chevauchement", "[shapes]") {
    auto inter = csg_inter(make_csg_left(), make_csg_right());
    auto hit = inter->hit(csg_ray(), 0.001, 1000.0);

    REQUIRE(hit.has_value());
    // Le premier hit de l'intersection est la surface de right (t≈3.68)
    REQUIRE(hit->t > 3.0);   // après l'entrée dans left
    REQUIRE(hit->t < 7.0);   // avant la sortie de left
}

TEST_CASE("CSGIntersection / ne frappe pas si les formes ne se chevauchent pas", "[shapes]") {
    auto a = std::make_shared<Sphere>(Vec3d(0,  0, -3), 1.0, dummy);
    auto b = std::make_shared<Sphere>(Vec3d(0,  0,  3), 1.0, dummy);  // côté opposé
    auto inter = csg_inter(a, b);
    auto hit = inter->hit(csg_ray(), 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("CSGDifference / frappe la surface de A hors de B", "[shapes]") {
    // left - right : la surface visible de left là où right ne couvre pas
    auto diff = csg_diff(make_csg_left(), make_csg_right());
    auto hit = diff->hit(csg_ray(), 0.001, 1000.0);

    REQUIRE(hit.has_value());
    // Le premier hit de A-B est l'entrée dans left (t≈3) qui est hors right (right entre à t≈3.68)
    REQUIRE(hit->t == Approx(3.0).epsilon(0.05));
}

TEST_CASE("CSGDifference / ne frappe pas si B contient entièrement A", "[shapes]") {
    // B (r=2) contient A (r=1) : A−B = vide
    // Ordre des événements : B_enter(t=3) → A_enter(t=4) → A_exit(t=6) → B_exit(t=7)
    // Aucun instant où on est dans A mais pas dans B
    auto a = std::make_shared<Sphere>(Vec3d(0, 0, 0), 1.0, dummy);
    auto b = std::make_shared<Sphere>(Vec3d(0, 0, 0), 2.0, dummy);
    auto diff = csg_diff(a, b);
    auto hit = diff->hit(csg_ray(), 0.001, 1000.0);
    REQUIRE_FALSE(hit.has_value());
}
