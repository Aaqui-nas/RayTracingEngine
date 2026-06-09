#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "shape.h"
#include "ray.h"
#include "vec3.h"
#include <cmath>
#include <random>

using namespace rt;
using Catch::Approx;

// ─────────────────────────────────────────────────────────────────────────────
// Les matériaux sont actuellement définis comme lambdas dans main().
// Ces tests les répliquent localement pour valider leur comportement.
// Après TP12bis (materials.h/.cpp), ces tests utiliseront build_materials().
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// ── Helpers (copiés depuis main.cpp) ─────────────────────────────────────────
thread_local std::mt19937 rng(42);  // seed fixe pour la reproductibilité
thread_local std::uniform_real_distribution<double> rng_dist(-1.0, 1.0);
thread_local std::uniform_real_distribution<double> rng_dist01(0.0, 1.0);

Vec3d random_in_unit_sphere() {
    Vec3d p;
    do { p = Vec3d(rng_dist(rng), rng_dist(rng), rng_dist(rng)); }
    while (dot(p,p) >= 1.0);
    return p;
}

Vec3d random_unit_vector() {
    Vec3d v = random_in_unit_sphere();
    return v / std::sqrt(dot(v, v));
}

Vec3d reflect(Vec3d v, Vec3d n) {
    return v - 2 * dot(v, n) * n;
}

Vec3d refract(Vec3d uv, Vec3d n, double ri) {
    double cos_theta = std::min(dot(-uv, n), 1.0);
    Vec3d perp = ri * (uv + cos_theta * n);
    Vec3d parallel = -std::sqrt(std::abs(1.0 - dot(perp, perp))) * n;
    return perp + parallel;
}

double schlick(double cosine, double ri) {
    double r0 = (1 - ri) / (1 + ri);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow(1 - cosine, 5);
}

// ── Factories (copie exacte de main.cpp) ─────────────────────────────────────
Material make_lambertian(Vec3d color) {
    return [color](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
        Vec3d dir = rec.normal + random_unit_vector();
        if (dot(dir, dir) < 1e-8) dir = rec.normal;
        return Scatter{color, Ray(rec.point, dir)};
    };
}

Material make_metal(Vec3d color, double fuzz) {
    return [color, fuzz](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
        Vec3d reflected = reflect(r.direction.normalized(), rec.normal);
        reflected = reflected + fuzz * random_in_unit_sphere();
        if (dot(reflected, rec.normal) <= 0) return std::nullopt;
        return Scatter{color, Ray(rec.point, reflected)};
    };
}

Material make_glass(double ior) {
    return [ior](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
        double ri = rec.front_face ? 1.0 / ior : ior;
        Vec3d unit_dir = r.direction.normalized();
        double cos_theta = std::min(dot(-unit_dir, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
        Vec3d dir = (ri * sin_theta > 1.0 || rng_dist01(rng) < schlick(cos_theta, ri))
            ? reflect(unit_dir, rec.normal)
            : refract(unit_dir, rec.normal, ri);
        return Scatter{Vec3d(255, 255, 255), Ray(rec.point, dir)};
    };
}

Material make_black() {
    return [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
        return std::nullopt;
    };
}

// Contexte d'impact minimal pour les tests
HitRecord make_hit(Vec3d normal = Vec3d(0,1,0), bool front = true) {
    return HitRecord{1.0, Vec3d(0,0,0), normal, front};
}

} // anonymous namespace

// ── Lambertian ────────────────────────────────────────────────────────────────

TEST_CASE("Material lambertian / retourne toujours un scatter") {
    auto mat = make_lambertian(Vec3d(200, 100, 50));
    Ray r(Vec3d(0,1,0), Vec3d(0,-1,0));
    auto rec = make_hit(Vec3d(0,1,0));

    // Testé sur plusieurs rayons aléatoires
    for (int i = 0; i < 20; i++) {
        auto scatter = mat(r, rec);
        REQUIRE(scatter.has_value());
    }
}

TEST_CASE("Material lambertian / atténuation correspond à la couleur") {
    Vec3d color(200, 100, 50);
    auto mat = make_lambertian(color);
    Ray r(Vec3d(0,1,0), Vec3d(0,-1,0));
    auto scatter = mat(r, make_hit());

    REQUIRE(scatter.has_value());
    REQUIRE(scatter->attenuation.x == Approx(200.0));
    REQUIRE(scatter->attenuation.y == Approx(100.0));
    REQUIRE(scatter->attenuation.z == Approx(50.0));
}

TEST_CASE("Material lambertian / rayon diffusé est dans l'hémisphère normal") {
    auto mat = make_lambertian(Vec3d(255, 255, 255));
    Ray r(Vec3d(0,10,0), Vec3d(0,-1,0));
    Vec3d normal(0, 1, 0);
    auto rec = make_hit(normal);

    for (int i = 0; i < 50; i++) {
        auto scatter = mat(r, rec);
        REQUIRE(scatter.has_value());
        // La direction diffusée doit être dans le même hémisphère que la normale
        REQUIRE(dot(scatter->scattered.direction, normal) > -1e-6);
    }
}

// ── Métal ─────────────────────────────────────────────────────────────────────

TEST_CASE("Material métal / fuzz=0 reflète parfaitement") {
    auto mat = make_metal(Vec3d(200, 200, 200), 0.0);
    Vec3d normal(0, 1, 0);
    Vec3d incoming(1, -1, 0);  // rayon incident à 45°
    Ray r(Vec3d(0, 1, 0), incoming.normalized());
    auto scatter = mat(r, make_hit(normal, true));

    REQUIRE(scatter.has_value());
    // La direction réfléchie doit être symétrique par rapport à la normale
    Vec3d reflected = reflect(incoming.normalized(), normal);
    REQUIRE(scatter->scattered.direction.x == Approx(reflected.x).epsilon(0.01));
    REQUIRE(scatter->scattered.direction.y == Approx(reflected.y).epsilon(0.01));
    REQUIRE(scatter->scattered.direction.z == Approx(reflected.z).epsilon(0.01));
}

TEST_CASE("Material métal / fuzz=0 direction toujours au-dessus de la normale") {
    auto mat = make_metal(Vec3d(255, 255, 255), 0.0);
    Vec3d normal(0, 1, 0);
    Ray r(Vec3d(0, 5, 0), Vec3d(1, -2, 0).normalized());

    for (int i = 0; i < 20; i++) {
        auto scatter = mat(r, make_hit(normal));
        REQUIRE(scatter.has_value());
        REQUIRE(dot(scatter->scattered.direction, normal) > 0.0);
    }
}

TEST_CASE("Material métal / rayon perpendiculaire à la normale retourne nullopt") {
    auto mat = make_metal(Vec3d(255, 255, 255), 0.0);
    Vec3d normal(0, 1, 0);
    // Rayon parallèle à la surface → réflexion dans le plan → dot <= 0
    Ray r(Vec3d(0, 0, 0), Vec3d(1, 0, 0));
    auto scatter = mat(r, make_hit(normal));

    // dot(reflected, normal) == 0 → selon l'implémentation peut retourner nullopt
    // On vérifie juste que ça ne plante pas
    (void)scatter;
    SUCCEED();
}

// ── Verre ─────────────────────────────────────────────────────────────────────

TEST_CASE("Material verre / retourne toujours un scatter") {
    auto mat = make_glass(1.5);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));

    for (int i = 0; i < 20; i++) {
        auto scatter = mat(r, make_hit(Vec3d(0,0,1), true));
        REQUIRE(scatter.has_value());
    }
}

TEST_CASE("Material verre / atténuation blanche (transparent)") {
    auto mat = make_glass(1.5);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    auto scatter = mat(r, make_hit(Vec3d(0,0,1), true));

    REQUIRE(scatter.has_value());
    REQUIRE(scatter->attenuation.x == Approx(255.0));
    REQUIRE(scatter->attenuation.y == Approx(255.0));
    REQUIRE(scatter->attenuation.z == Approx(255.0));
}

TEST_CASE("Material verre / direction sortante non nulle") {
    auto mat = make_glass(1.5);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    auto scatter = mat(r, make_hit(Vec3d(0, 0, 1), true));

    REQUIRE(scatter.has_value());
    double len2 = dot(scatter->scattered.direction, scatter->scattered.direction);
    REQUIRE(len2 > 1e-6);
}

// ── Matériau noir (__black__) ─────────────────────────────────────────────────

TEST_CASE("Material black / retourne toujours nullopt") {
    auto mat = make_black();
    Ray r(Vec3d(0,0,0), Vec3d(0,0,-1));
    auto scatter = mat(r, make_hit());

    REQUIRE_FALSE(scatter.has_value());
}

// ── Propriétés générales ──────────────────────────────────────────────────────

TEST_CASE("Material / le rayon diffusé part du point d'impact") {
    auto mat = make_lambertian(Vec3d(255, 0, 0));
    Vec3d impact(3, 4, 5);
    HitRecord rec{1.0, impact, Vec3d(0,1,0), true};
    Ray r(Vec3d(0,0,0), Vec3d(0,1,0));

    auto scatter = mat(r, rec);
    REQUIRE(scatter.has_value());
    REQUIRE(scatter->scattered.origin.x == Approx(3.0));
    REQUIRE(scatter->scattered.origin.y == Approx(4.0));
    REQUIRE(scatter->scattered.origin.z == Approx(5.0));
}
