#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <vector>
#include "core/mat3.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "materials/texture.h"
#include "materials/materials.h"

using namespace rt;
using Catch::Approx;

static Material dummy_mat = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return std::nullopt;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Mat3
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Mat3 / identité — vecteur inchangé") {
    Mat3 I{Vec3d(1,0,0), Vec3d(0,1,0), Vec3d(0,0,1)};
    Vec3d v(2, -3, 5);
    Vec3d r = I * v;
    REQUIRE(r.x == Approx(2.0));
    REQUIRE(r.y == Approx(-3.0));
    REQUIRE(r.z == Approx(5.0));
}

TEST_CASE("Mat3 / transformation des vecteurs de base") {
    // col0 → reçoit x, col1 → reçoit y, col2 → reçoit z
    Mat3 M{Vec3d(0,1,0), Vec3d(0,0,1), Vec3d(1,0,0)};

    Vec3d rx = M * Vec3d(1, 0, 0);
    REQUIRE(rx.x == Approx(0.0)); REQUIRE(rx.y == Approx(1.0)); REQUIRE(rx.z == Approx(0.0));

    Vec3d ry = M * Vec3d(0, 1, 0);
    REQUIRE(ry.x == Approx(0.0)); REQUIRE(ry.y == Approx(0.0)); REQUIRE(ry.z == Approx(1.0));

    Vec3d rz = M * Vec3d(0, 0, 1);
    REQUIRE(rz.x == Approx(1.0)); REQUIRE(rz.y == Approx(0.0)); REQUIRE(rz.z == Approx(0.0));
}

TEST_CASE("Mat3 / TBN — normale plate (0,0,1) → normale géométrique") {
    // La normal map plate stocke (0,0,1) en espace tangent.
    // La TBN doit retourner exactement la normale géométrique.
    Vec3d T(1, 0, 0), B(0, 1, 0), N(0, 0, 1);
    Mat3 tbn{T, B, N};

    Vec3d result = tbn * Vec3d(0, 0, 1);
    REQUIRE(result.x == Approx(N.x));
    REQUIRE(result.y == Approx(N.y));
    REQUIRE(result.z == Approx(N.z));
}

TEST_CASE("Mat3 / TBN — normale inclinée vers T") {
    Vec3d T(1, 0, 0), B(0, 1, 0), N(0, 0, 1);
    Mat3 tbn{T, B, N};

    // (1,0,0) en tangent space → T en world space
    Vec3d result = tbn * Vec3d(1, 0, 0);
    REQUIRE(result.x == Approx(T.x));
    REQUIRE(result.y == Approx(T.y));
    REQUIRE(result.z == Approx(T.z));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tangent sur Sphere
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Sphere / hit() fournit un tangent unitaire") {
    Sphere s(Vec3d(0, 0, 0), 1.0, dummy_mat);
    Ray r(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(dot(hit->tangent, hit->tangent) == Approx(1.0).epsilon(1e-6));
}

TEST_CASE("Sphere / tangent perpendiculaire à la normale") {
    Sphere s(Vec3d(0, 0, 0), 1.0, dummy_mat);
    std::vector<Vec3d> dirs = {
        Vec3d(0, 0, 1), Vec3d(1, 0, 0), Vec3d(-1, 0, 0), Vec3d(0, 0, -1),
        Vec3d(1, 1, 0).normalized(), Vec3d(1, 0, 1).normalized()
    };
    for (auto& d : dirs) {
        Ray r(d * 5, -d);
        auto hit = s.hit(r, 0.001, 1000.0);
        REQUIRE(hit.has_value());
        REQUIRE(dot(hit->tangent, hit->normal) == Approx(0.0).margin(1e-6));
    }
}

TEST_CASE("Sphere / tangent unitaire sur multiples directions") {
    Sphere s(Vec3d(0, 0, 0), 1.0, dummy_mat);
    std::vector<Vec3d> dirs = {
        Vec3d(1, 0, 0), Vec3d(0, 0, -1), Vec3d(1, 0, 1).normalized(),
        Vec3d(0.5, 0.3, 0.8).normalized()
    };
    for (auto& d : dirs) {
        Ray r(d * 5, -d);
        auto hit = s.hit(r, 0.001, 1000.0);
        REQUIRE(hit.has_value());
        double len2 = dot(hit->tangent, hit->tangent);
        REQUIRE(len2 == Approx(1.0).epsilon(1e-6));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tangent sur Triangle
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Triangle / hit() fournit un tangent unitaire") {
    Vec3d A(0, 0, -2), B(1, 0, -2), C(0, 1, -2);
    Vec3d uvA(0, 0, 0), uvB(1, 0, 0), uvC(0, 1, 0);
    Triangle tri(A, B, C, uvA, uvB, uvC, dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));
    auto hit = tri.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(dot(hit->tangent, hit->tangent) == Approx(1.0).epsilon(1e-6));
}

TEST_CASE("Triangle / tangent perpendiculaire à la normale") {
    Vec3d A(0, 0, -2), B(1, 0, -2), C(0, 1, -2);
    Vec3d uvA(0, 0, 0), uvB(1, 0, 0), uvC(0, 1, 0);
    Triangle tri(A, B, C, uvA, uvB, uvC, dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));
    auto hit = tri.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(dot(hit->tangent, hit->normal) == Approx(0.0).margin(1e-6));
}

TEST_CASE("Triangle / tangent aligné avec la direction U des UV") {
    // Triangle dans z=-2 : A=(0,0,-2) uv=(0,0), B=(1,0,-2) uv=(1,0), C=(0,1,-2) uv=(0,1)
    // dPos1 = B-A = (1,0,0), dUV1 = (1,0) → la direction U correspond à (1,0,0)
    // Donc T doit être aligné avec l'axe X.
    Vec3d A(0, 0, -2), B(1, 0, -2), C(0, 1, -2);
    Vec3d uvA(0, 0, 0), uvB(1, 0, 0), uvC(0, 1, 0);
    Triangle tri(A, B, C, uvA, uvB, uvC, dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));
    auto hit = tri.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    // |tangent.x| = 1, tangent.y = tangent.z = 0
    REQUIRE(std::abs(hit->tangent.x) == Approx(1.0).epsilon(1e-6));
    REQUIRE(hit->tangent.y == Approx(0.0).margin(1e-6));
    REQUIRE(hit->tangent.z == Approx(0.0).margin(1e-6));
}

// ═══════════════════════════════════════════════════════════════════════════════
// BumpTexture
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("BumpTexture / retourne un gradient fini") {
    BumpTexture bump(4.0, 1.0);
    std::vector<Vec3d> pts = {{0,0,0}, {1,0,0}, {0.5,0.5,0}, {1.3,2.1,0.7}};
    for (auto& p : pts) {
        Vec3d g = bump.sample(0.5, 0.5, p);
        REQUIRE(std::isfinite(g.x));
        REQUIRE(std::isfinite(g.y));
        REQUIRE(std::isfinite(g.z));
    }
}

TEST_CASE("BumpTexture / des points différents donnent des gradients différents") {
    BumpTexture bump(4.0, 1.0);
    Vec3d g1 = bump.sample(0.5, 0.5, Vec3d(0, 0, 0));
    Vec3d g2 = bump.sample(0.5, 0.5, Vec3d(10, 10, 10));
    REQUIRE(dot(g1 - g2, g1 - g2) > 1e-6);
}

TEST_CASE("BumpTexture / strength=0 → gradient nul") {
    BumpTexture bump(4.0, 0.0);
    for (double u : {0.0, 0.25, 0.5, 0.75}) {
        Vec3d g = bump.sample(u, u, Vec3d(u, u * 0.5, u * 0.3));
        REQUIRE(g.x == Approx(0.0).margin(1e-6));
        REQUIRE(g.y == Approx(0.0).margin(1e-6));
        REQUIRE(g.z == Approx(0.0).margin(1e-6));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// normal_mapped() — décorateur de matériau
// ═══════════════════════════════════════════════════════════════════════════════

// Matériau espion : retourne la normale courante encodée dans l'atténuation [0,255]
// → permet d'inspecter quelle normale a été utilisée par le matériau de base.
static Material capture_normal = [](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
    return Scatter{(rec.normal + Vec3d(1, 1, 1)) * 127.5, Ray(rec.point, rec.normal)};
};

TEST_CASE("normal_mapped / normale plate → normale ≈ normale géométrique") {
    // BumpTexture(strength=0) retourne (0,0,1) en tangent space.
    // Après TBN, on doit retrouver la normale géométrique.
    auto flat = std::make_shared<BumpTexture>(4.0, 0.0);
    auto mat  = normal_mapped(capture_normal, flat);

    Sphere s(Vec3d(0, 0, 0), 1.0, mat);
    Ray r(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());

    Vec3d geom_normal = hit->normal;  // (0, 0, 1) attendu pour ce rayon
    auto scatter = hit->material(r, *hit);
    REQUIRE(scatter.has_value());

    Vec3d returned = scatter->attenuation / 127.5 - Vec3d(1, 1, 1);
    REQUIRE(returned.x == Approx(geom_normal.x).margin(0.01));
    REQUIRE(returned.y == Approx(geom_normal.y).margin(0.01));
    REQUIRE(returned.z == Approx(geom_normal.z).epsilon(0.01));
}

TEST_CASE("normal_mapped / normale perturbée reste unitaire") {
    auto bump = std::make_shared<BumpTexture>(4.0, 2.0);
    auto mat  = normal_mapped(capture_normal, bump);

    Sphere s(Vec3d(0, 0, 0), 1.0, mat);
    std::vector<Vec3d> dirs = {
        Vec3d(0, 0, 1), Vec3d(1, 0, 0), Vec3d(0, 1, 0), Vec3d(1, 1, 1).normalized()
    };
    for (auto& d : dirs) {
        Ray r(d * 5, -d);
        auto hit = s.hit(r, 0.001, 1000.0);
        if (!hit.has_value()) continue;
        auto scatter = hit->material(r, *hit);
        REQUIRE(scatter.has_value());
        Vec3d n = scatter->attenuation / 127.5 - Vec3d(1, 1, 1);
        REQUIRE(n.length() == Approx(1.0).epsilon(0.01));
    }
}

TEST_CASE("normal_mapped / décorateur ne modifie pas les autres champs du HitRecord") {
    // Après decoration, rec.point, rec.u, rec.v doivent être inchangés.
    // On vérifie via un matériau espion qui capture rec.u.
    double captured_u = -1.0;
    Material capture_u = [&captured_u](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
        captured_u = rec.u;
        return std::nullopt;
    };

    auto bump = std::make_shared<BumpTexture>(4.0, 1.0);
    auto mat  = normal_mapped(capture_u, bump);

    Sphere s(Vec3d(0, 0, 0), 1.0, mat);
    Ray r(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());

    double expected_u = hit->u;
    hit->material(r, *hit);
    REQUIRE(captured_u == Approx(expected_u));
}
