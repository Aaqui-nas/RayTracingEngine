#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <memory>
#include "sphere.h"
#include "plane.h"
#include "triangle.h"
#include "mesh.h"

using namespace rt;
using Catch::Approx;

// Matériau neutre pour les tests — les tests de shapes ne portent pas sur les matériaux
static Material dummy_mat = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return Scatter{Vec3d(255, 255, 255), Ray(Vec3d(0,0,0), Vec3d(0,0,-1))};
};

// ═══════════════════════════════════════════════════════════════════════════════
// Sphere
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Sphere / rayon centré frappe en face avant") {
    Sphere s(Vec3d(0, 0, -2), 0.5, dummy_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));

    auto hit = s.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->t      == Approx(1.5).epsilon(0.01));
    REQUIRE(hit->front_face == true);
    REQUIRE(dot(hit->normal, r.direction) < 0.0);  // normale opposée à la direction
}

TEST_CASE("Sphere / rayon tangent frappe la surface") {
    Sphere s(Vec3d(0, 0, -2), 1.0, dummy_mat);
    // rayon passant exactement par le bord de la sphère (y = 1.0)
    Ray r(Vec3d(0, 1, 0), Vec3d(0, 0, -1));

    auto hit = s.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
}

TEST_CASE("Sphere / rayon à côté ne frappe pas") {
    Sphere s(Vec3d(0, 0, -2), 0.5, dummy_mat);
    Ray r(Vec3d(2, 0, 0), Vec3d(0, 0, -1));  // passant loin à droite

    auto hit = s.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Sphere / rayon à l'envers (derrière l'origine) ne frappe pas") {
    Sphere s(Vec3d(0, 0, 2), 0.5, dummy_mat);   // sphère en +z
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));       // rayon vers -z

    auto hit = s.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Sphere / origine du rayon à l'intérieur — face arrière") {
    // Le rayon démarre à l'intérieur de la sphère
    Sphere s(Vec3d(0, 0, 0), 1.0, dummy_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));

    auto hit = s.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->front_face == false);
    REQUIRE(dot(hit->normal, r.direction) < 0.0);  // normale toujours contra-directionnelle
}

TEST_CASE("Sphere / rejet par tmin — intersection trop proche") {
    Sphere s(Vec3d(0, 0, -2), 0.5, dummy_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));

    // tmin > 1.5, donc le hit devrait être rejeté
    auto hit = s.hit(r, 10.0, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Sphere / rejet par tmax — intersection trop loin") {
    Sphere s(Vec3d(0, 0, -20), 0.5, dummy_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));

    auto hit = s.hit(r, 0.001, 5.0);  // tmax = 5, sphère à t ≈ 19.5

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Sphere / le point d'impact est sur la surface") {
    Sphere s(Vec3d(0, 0, -2), 0.5, dummy_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));

    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());

    // distance entre le point d'impact et le centre == rayon
    Vec3d diff = hit->point - Vec3d(0, 0, -2);
    double dist = std::sqrt(dot(diff, diff));
    REQUIRE(dist == Approx(0.5).epsilon(1e-6));
}

TEST_CASE("Sphere / la normale pointe vers l'extérieur") {
    Sphere s(Vec3d(0, 0, -2), 0.5, dummy_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));

    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());

    // normale est unitaire
    REQUIRE(dot(hit->normal, hit->normal) == Approx(1.0).epsilon(1e-6));

    // normale pointe vers la source (dot avec vecteur centre→point > 0)
    Vec3d outward = (hit->point - Vec3d(0, 0, -2)).normalized();
    REQUIRE(dot(outward, hit->normal) == Approx(1.0).epsilon(1e-4));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Plane
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Plane / rayon perpendiculaire descend vers y=0") {
    Plane p(Vec3d(0, 0, 0), Vec3d(0, 1, 0), dummy_mat);
    Ray r(Vec3d(0, 5, 0), Vec3d(0, -1, 0));

    auto hit = p.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->t == Approx(5.0));
    REQUIRE(hit->point.y == Approx(0.0).margin(1e-9));
}

TEST_CASE("Plane / rayon parallèle au plan ne frappe pas") {
    Plane p(Vec3d(0, 0, 0), Vec3d(0, 1, 0), dummy_mat);
    Ray r(Vec3d(0, 1, 0), Vec3d(1, 0, 0));  // rayon horizontal

    auto hit = p.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Plane / rayon depuis en-dessous — face arrière") {
    Plane p(Vec3d(0, 0, 0), Vec3d(0, 1, 0), dummy_mat);
    Ray r(Vec3d(0, -5, 0), Vec3d(0, 1, 0));  // monte de bas en haut

    auto hit = p.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->front_face == false);
    REQUIRE(dot(hit->normal, r.direction) < 0.0);
}

TEST_CASE("Plane / rejet par tmin") {
    Plane p(Vec3d(0, 0, 0), Vec3d(0, 1, 0), dummy_mat);
    Ray r(Vec3d(0, 1, 0), Vec3d(0, -1, 0));  // t = 1

    auto hit = p.hit(r, 2.0, 1000.0);  // tmin > 1

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Plane / la normale est unitaire") {
    Plane p(Vec3d(0, 0, 0), Vec3d(0, 3, 0), dummy_mat);  // normale non normalisée
    Ray r(Vec3d(0, 5, 0), Vec3d(0, -1, 0));

    auto hit = p.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());

    double len = std::sqrt(dot(hit->normal, hit->normal));
    REQUIRE(len == Approx(1.0).epsilon(1e-6));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Triangle
// Triangle de test : A=(0,0,-2), B=(1,0,-2), C=(0,1,-2) — plan z=-2, face vers +z
// ═══════════════════════════════════════════════════════════════════════════════

static Vec3d tA(0, 0, -2), tB(1, 0, -2), tC(0, 1, -2);

TEST_CASE("Triangle / rayon frappe en face avant") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));

    auto hit = tri.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->t == Approx(2.0).epsilon(1e-6));
    REQUIRE(hit->front_face == true);
    REQUIRE(hit->point.z == Approx(-2.0).margin(1e-9));
}

TEST_CASE("Triangle / rayon frappe en face arrière") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(0.2, 0.2, -4), Vec3d(0, 0, 1));  // vient de derrière

    auto hit = tri.hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->front_face == false);
    REQUIRE(dot(hit->normal, r.direction) < 0.0);
}

TEST_CASE("Triangle / rayon parallèle ne frappe pas") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(1, 0, 0));  // parallèle au plan z=-2

    auto hit = tri.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Triangle / rayon manque par u") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(2, 0.1, 0), Vec3d(0, 0, -1));  // u > 1

    auto hit = tri.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Triangle / rayon manque par v") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(0.1, 2, 0), Vec3d(0, 0, -1));  // v > 1

    auto hit = tri.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Triangle / rayon manque par u+v") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(0.6, 0.6, 0), Vec3d(0, 0, -1));  // u+v > 1

    auto hit = tri.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Triangle / rejet par tmin") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));  // t = 2

    auto hit = tri.hit(r, 3.0, 1000.0);  // tmin > 2

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Triangle / rejet par tmax") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));  // t = 2

    auto hit = tri.hit(r, 0.001, 1.0);  // tmax < 2

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Triangle / la normale est unitaire") {
    Triangle tri(tA, tB, tC, dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));

    auto hit = tri.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());

    double len = std::sqrt(dot(hit->normal, hit->normal));
    REQUIRE(len == Approx(1.0).epsilon(1e-6));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Mesh
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Mesh / rayon frappe le seul triangle") {
    auto mesh = std::make_shared<Mesh>(dummy_mat);
    mesh->add(Triangle(tA, tB, tC, dummy_mat));
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));

    auto hit = mesh->hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->t == Approx(2.0).epsilon(1e-6));
}

TEST_CASE("Mesh / retourne le triangle le plus proche") {
    auto mesh = std::make_shared<Mesh>(dummy_mat);
    // Triangle proche à z=-2, triangle loin à z=-5
    mesh->add(Triangle(Vec3d(0,0,-5), Vec3d(1,0,-5), Vec3d(0,1,-5), dummy_mat));
    mesh->add(Triangle(tA, tB, tC, dummy_mat));  // z=-2, plus proche
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));

    auto hit = mesh->hit(r, 0.001, 1000.0);

    REQUIRE(hit.has_value());
    REQUIRE(hit->t == Approx(2.0).epsilon(1e-6));  // le plus proche, pas le plus loin
}

TEST_CASE("Mesh / rayon manque tous les triangles") {
    auto mesh = std::make_shared<Mesh>(dummy_mat);
    mesh->add(Triangle(tA, tB, tC, dummy_mat));
    Ray r(Vec3d(5, 5, 0), Vec3d(0, 0, -1));  // loin du triangle

    auto hit = mesh->hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}

TEST_CASE("Mesh / mesh vide ne frappe pas") {
    auto mesh = std::make_shared<Mesh>(dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));

    auto hit = mesh->hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(hit.has_value());
}
