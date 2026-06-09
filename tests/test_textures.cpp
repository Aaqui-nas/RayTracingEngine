#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include "materials/texture.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"

using namespace rt;
using Catch::Approx;

static Material dummy_mat = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return std::nullopt;
};

// ═══════════════════════════════════════════════════════════════════════════════
// SolidColor
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("SolidColor / retourne toujours la même couleur") {
    SolidColor tex(Vec3d(100, 150, 200));
    REQUIRE(tex.sample(0.0, 0.0, Vec3d(0,0,0)).x == Approx(100));
    REQUIRE(tex.sample(0.5, 0.5, Vec3d(0,0,0)).y == Approx(150));
    REQUIRE(tex.sample(1.0, 1.0, Vec3d(1,1,1)).z == Approx(200));
}

TEST_CASE("SolidColor / indépendant des coordonnées UV et du point") {
    SolidColor tex(Vec3d(255, 0, 0));
    Vec3d c1 = tex.sample(0.0, 0.0, Vec3d(0,0,0));
    Vec3d c2 = tex.sample(0.7, 0.3, Vec3d(5,-2,3));
    REQUIRE(c1.x == Approx(c2.x));
    REQUIRE(c1.y == Approx(c2.y));
    REQUIRE(c1.z == Approx(c2.z));
}

// ═══════════════════════════════════════════════════════════════════════════════
// CheckerTexture
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("CheckerTexture / alterne entre even et odd") {
    Vec3d white(255, 255, 255), black(0, 0, 0);
    CheckerTexture tex(white, black, 2.0);

    // u=0.1, v=0.1 → case (0, 0) → somme paire → white
    Vec3d c1 = tex.sample(0.1, 0.1, Vec3d(0,0,0));
    // u=0.6, v=0.1 → case (1, 0) → somme impaire → black
    Vec3d c2 = tex.sample(0.6, 0.1, Vec3d(0,0,0));

    REQUIRE(c1.x == Approx(255));
    REQUIRE(c2.x == Approx(0));
}

TEST_CASE("CheckerTexture / scale=1 — 1 case sur toute la surface") {
    CheckerTexture tex(Vec3d(255, 0, 0), Vec3d(0, 0, 255), 1.0);
    // (0.1, 0.1) → case (0,0) → even
    Vec3d c = tex.sample(0.1, 0.1, Vec3d(0,0,0));
    REQUIRE(c.x == Approx(255));
    REQUIRE(c.z == Approx(0));
}

TEST_CASE("CheckerTexture / coins opposés de couleurs différentes") {
    CheckerTexture tex(Vec3d(255, 255, 255), Vec3d(0, 0, 0), 4.0);
    Vec3d coin1 = tex.sample(0.1, 0.1, Vec3d(0,0,0));  // case (0,0) → even
    Vec3d coin2 = tex.sample(0.4, 0.1, Vec3d(0,0,0));  // case (1,0) → odd
    REQUIRE(coin1.x != Approx(coin2.x));
}

// ═══════════════════════════════════════════════════════════════════════════════
// UV sur Sphere
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Sphere UV / pôle sud (0,-1,0) → v=0") {
    Sphere s(Vec3d(0,0,0), 1.0, dummy_mat);
    Ray r(Vec3d(0, -5, 0), Vec3d(0, 1, 0));
    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->v == Approx(0.0).margin(1e-4));
    REQUIRE(hit->u == Approx(0.5).margin(1e-4));
}

TEST_CASE("Sphere UV / pôle nord (0,+1,0) → v=1") {
    Sphere s(Vec3d(0,0,0), 1.0, dummy_mat);
    Ray r(Vec3d(0, 5, 0), Vec3d(0, -1, 0));
    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->v == Approx(1.0).margin(1e-4));
    REQUIRE(hit->u == Approx(0.5).margin(1e-4));
}

TEST_CASE("Sphere UV / devant (0,0,+1) → u=0.25") {
    Sphere s(Vec3d(0,0,0), 1.0, dummy_mat);
    Ray r(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->u == Approx(0.25).margin(1e-4));
    REQUIRE(hit->v == Approx(0.5).margin(1e-4));
}

TEST_CASE("Sphere UV / derrière (0,0,-1) → u=0.75") {
    Sphere s(Vec3d(0,0,0), 1.0, dummy_mat);
    Ray r(Vec3d(0, 0, -5), Vec3d(0, 0, 1));
    auto hit = s.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->u == Approx(0.75).margin(1e-4));
    REQUIRE(hit->v == Approx(0.5).margin(1e-4));
}

TEST_CASE("Sphere UV / valeurs dans [0, 1]") {
    Sphere s(Vec3d(0,0,0), 1.0, dummy_mat);
    // Quelques rayons dans différentes directions
    std::vector<Vec3d> dirs = {
        Vec3d(1, 0, 0), Vec3d(-1, 0, 0), Vec3d(0, 1, 0),
        Vec3d(0, -1, 0), Vec3d(1, 1, 0).normalized(), Vec3d(1, 1, 1).normalized()
    };
    for (auto& d : dirs) {
        Ray r(d * 5, -d);
        auto hit = s.hit(r, 0.001, 1000.0);
        REQUIRE(hit.has_value());
        REQUIRE(hit->u >= 0.0);
        REQUIRE(hit->u <= 1.0);
        REQUIRE(hit->v >= 0.0);
        REQUIRE(hit->v <= 1.0);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// UV sur Triangle
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Triangle UV / rayon au sommet A retourne uv_A") {
    Vec3d A(0, 0, -2), B(1, 0, -2), C(0, 1, -2);
    Vec3d uvA(0, 0, 0), uvB(1, 0, 0), uvC(0, 1, 0);
    Triangle tri(A, B, C, uvA, uvB, uvC, dummy_mat);
    // Rayon passant juste à côté de A (epsilon barycentrique)
    Ray r(Vec3d(0.01, 0.01, 0), Vec3d(0, 0, -1));
    auto hit = tri.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->u == Approx(0.0).margin(0.02));
    REQUIRE(hit->v == Approx(0.0).margin(0.02));
}

TEST_CASE("Triangle UV / centroïde → moyenne des UV sommets") {
    Vec3d A(0, 0, -2), B(1, 0, -2), C(0, 1, -2);
    Vec3d uvA(0, 0, 0), uvB(1, 0, 0), uvC(0, 1, 0);
    Triangle tri(A, B, C, uvA, uvB, uvC, dummy_mat);
    // Centroïde = (A+B+C)/3 = (1/3, 1/3, -2)
    Ray r(Vec3d(1.0/3.0, 1.0/3.0, 0), Vec3d(0, 0, -1));
    auto hit = tri.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->u == Approx(1.0/3.0).epsilon(0.01));
    REQUIRE(hit->v == Approx(1.0/3.0).epsilon(0.01));
}

TEST_CASE("Triangle UV / constructeur sans UV — valeurs par défaut valides") {
    // L'ancien constructeur sans UV doit toujours compiler et retourner u,v ∈ [0,1]
    Triangle tri(Vec3d(0,0,-2), Vec3d(1,0,-2), Vec3d(0,1,-2), dummy_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));
    auto hit = tri.hit(r, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    REQUIRE(hit->u >= 0.0);
    REQUIRE(hit->u <= 1.0);
    REQUIRE(hit->v >= 0.0);
    REQUIRE(hit->v <= 1.0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// ImageTexture
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("ImageTexture / pixel correct dans une image 2x2") {
    // Image 2x2 :  rouge  | vert
    //              bleu   | blanc
    int w = 2, h = 2;
    std::vector<uint8_t> data = {
        255, 0,   0,    // (0,0) rouge
        0,   255, 0,    // (1,0) vert
        0,   0,   255,  // (0,1) bleu
        255, 255, 255   // (1,1) blanc
    };
    ImageTexture tex(data, w, h);

    Vec3d rouge = tex.sample(0.1, 0.1, Vec3d(0,0,0));  // coin (0,0)
    REQUIRE(rouge.x == Approx(255));
    REQUIRE(rouge.y == Approx(0));

    Vec3d vert = tex.sample(0.9, 0.1, Vec3d(0,0,0));   // coin (1,0)
    REQUIRE(vert.x == Approx(0));
    REQUIRE(vert.y == Approx(255));

    Vec3d bleu = tex.sample(0.1, 0.9, Vec3d(0,0,0));   // coin (0,1)
    REQUIRE(bleu.z == Approx(255));
    REQUIRE(bleu.x == Approx(0));
}

TEST_CASE("ImageTexture / UV wrapping — u=1.1 revient à u=0.1") {
    int w = 2, h = 2;
    std::vector<uint8_t> data = {
        100, 0, 0,   200, 0, 0,
        100, 0, 0,   200, 0, 0
    };
    ImageTexture tex(data, w, h);
    Vec3d c1 = tex.sample(0.1, 0.1, Vec3d(0,0,0));
    Vec3d c2 = tex.sample(1.1, 0.1, Vec3d(0,0,0));
    REQUIRE(c1.x == Approx(c2.x));
}

// ═══════════════════════════════════════════════════════════════════════════════
// PerlinNoise
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PerlinNoise / valeurs dans [-1, 1]") {
    PerlinNoise pn;
    std::vector<Vec3d> points = {
        {0,0,0}, {1,0,0}, {0,1,0}, {0,0,1},
        {0.5,0.5,0.5}, {1.5,2.3,0.7}, {-1,0.3,2.1}
    };
    for (auto& p : points) {
        double n = pn.noise(p);
        REQUIRE(n >= -1.0);
        REQUIRE(n <=  1.0);
    }
}

TEST_CASE("PerlinNoise / bruit lisse — points proches ont des valeurs proches") {
    PerlinNoise pn;
    Vec3d p(1.0, 2.0, 3.0);
    double n1 = pn.noise(p);
    double n2 = pn.noise(p + Vec3d(0.001, 0, 0));
    REQUIRE(std::abs(n1 - n2) < 0.1);  // continuité : variation < 10% pour Δ=0.001
}

TEST_CASE("PerlinNoise / déterministe — même point → même valeur") {
    PerlinNoise pn;
    Vec3d p(1.23, 4.56, 7.89);
    double n1 = pn.noise(p);
    double n2 = pn.noise(p);
    REQUIRE(n1 == Approx(n2));
}

TEST_CASE("PerlinNoise / turbulence dans [0, 1]") {
    PerlinNoise pn;
    std::vector<Vec3d> points = {
        {0,0,0}, {1.5,2.3,0.7}, {-0.5,1.0,3.3}
    };
    for (auto& p : points) {
        double t = pn.turbulence(p, 7);
        REQUIRE(t >= 0.0);
        REQUIRE(t <= 1.0);
    }
}

TEST_CASE("PerlinTexture / sample retourne une couleur valide") {
    auto tex = std::make_shared<PerlinTexture>(Vec3d(255, 255, 255), 4.0);
    Vec3d c = tex->sample(0.5, 0.5, Vec3d(1, 2, 3));
    REQUIRE(c.x >= 0);
    REQUIRE(c.x <= 255);
    REQUIRE(c.y >= 0);
    REQUIRE(c.z >= 0);
}
