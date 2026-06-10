#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include "materials/env_map.h"

using namespace rt;
using Catch::Approx;

// ═══════════════════════════════════════════════════════════════════════════════
// rgbe_to_float
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("rgbe_to_float / E=0 → noir absolu") {
    Vec3d c = rgbe_to_float(255, 255, 255, 0);
    REQUIRE(c.x == Approx(0.0));
    REQUIRE(c.y == Approx(0.0));
    REQUIRE(c.z == Approx(0.0));
}

TEST_CASE("rgbe_to_float / (0,0,0,136) → (0.5, 0.5, 0.5)") {
    // E=136 → f = 2^(136-128) = 256
    // v = (0+0.5)/256 * 256 = 0.5
    Vec3d c = rgbe_to_float(0, 0, 0, 136);
    REQUIRE(c.x == Approx(0.5).epsilon(1e-4));
    REQUIRE(c.y == Approx(0.5).epsilon(1e-4));
    REQUIRE(c.z == Approx(0.5).epsilon(1e-4));
}

TEST_CASE("rgbe_to_float / (255,0,0,128) → rouge proche de 1, autres très faibles") {
    // E=128 → f = 2^0 = 1
    // r = (255+0.5)/256 ≈ 0.998, g = b = (0+0.5)/256 ≈ 0.002
    Vec3d c = rgbe_to_float(255, 0, 0, 128);
    REQUIRE(c.x == Approx(255.5 / 256.0).epsilon(1e-4));
    REQUIRE(c.y == Approx(0.5   / 256.0).epsilon(1e-4));
    REQUIRE(c.z == Approx(0.5   / 256.0).epsilon(1e-4));
}

TEST_CASE("rgbe_to_float / (128,128,128,129) → valeurs légèrement au-dessus de 1") {
    // E=129 → f = 2^1 = 2
    // v = (128+0.5)/256 * 2 = 257/256 ≈ 1.004
    Vec3d c = rgbe_to_float(128, 128, 128, 129);
    double expected = (128.0 + 0.5) / 256.0 * 2.0;
    REQUIRE(c.x == Approx(expected).epsilon(1e-4));
    REQUIRE(c.y == Approx(expected).epsilon(1e-4));
    REQUIRE(c.z == Approx(expected).epsilon(1e-4));
    REQUIRE(c.x > 1.0);  // HDR : dépasse la plage LDR
}

TEST_CASE("rgbe_to_float / exposant élevé → valeurs très grandes") {
    // E=200 → f = 2^72
    // Vérifie juste que c'est fini et > 1
    Vec3d c = rgbe_to_float(128, 64, 32, 200);
    REQUIRE(std::isfinite(c.x));
    REQUIRE(std::isfinite(c.y));
    REQUIRE(std::isfinite(c.z));
    REQUIRE(c.x > 1.0);
}

TEST_CASE("rgbe_to_float / composantes respectent les proportions RGB") {
    // (204, 128, 77, 128) : les ratios r:g:b doivent être ≈ 204:128:77
    Vec3d c = rgbe_to_float(204, 128, 77, 128);
    REQUIRE(c.x > c.y);
    REQUIRE(c.y > c.z);
    double ratio_rg = c.x / c.y;
    REQUIRE(ratio_rg == Approx((204.5) / (128.5)).epsilon(0.01));
}

// ═══════════════════════════════════════════════════════════════════════════════
// EnvMap — construction et sample
// ═══════════════════════════════════════════════════════════════════════════════

static EnvMap make_uniform_map(int w, int h, float val = 1.0f) {
    std::vector<float> data(w * h * 3, val);
    return EnvMap(std::move(data), w, h);
}

TEST_CASE("EnvMap / constructeur par défaut → empty()") {
    EnvMap e;
    REQUIRE(e.empty());
    REQUIRE(e.width()  == 0);
    REQUIRE(e.height() == 0);
}

TEST_CASE("EnvMap / constructeur avec données → dimensions correctes") {
    EnvMap e = make_uniform_map(16, 8);
    REQUIRE_FALSE(e.empty());
    REQUIRE(e.width()  == 16);
    REQUIRE(e.height() == 8);
}

TEST_CASE("EnvMap / sample sur carte uniforme → même valeur partout") {
    EnvMap e = make_uniform_map(4, 4, 2.5f);
    Vec3d c0 = e.sample(0.1, 0.1);
    Vec3d c1 = e.sample(0.5, 0.5);
    Vec3d c2 = e.sample(0.9, 0.9);
    REQUIRE(c0.x == Approx(c1.x).epsilon(0.01));
    REQUIRE(c1.x == Approx(c2.x).epsilon(0.01));
    REQUIRE(c0.y == Approx(c1.y).epsilon(0.01));
}

TEST_CASE("EnvMap / sample sur image 2x2 — coin haut-gauche") {
    // Layout : rouge | vert
    //          bleu  | blanc
    std::vector<float> data = {
        1.0f, 0.0f, 0.0f,   // (0,0) rouge
        0.0f, 1.0f, 0.0f,   // (1,0) vert
        0.0f, 0.0f, 1.0f,   // (0,1) bleu
        1.0f, 1.0f, 1.0f,   // (1,1) blanc
    };
    EnvMap e(std::move(data), 2, 2);

    // Très près du coin (0,0) → rouge
    Vec3d rouge = e.sample(0.01, 0.01);
    REQUIRE(rouge.x > rouge.y);
    REQUIRE(rouge.x > rouge.z);
}

TEST_CASE("EnvMap / sample interpolation bilinéaire — centre 2x2 ≈ moyenne") {
    // Image 2×2 : toutes les couleurs différentes
    // sample(0.5, 0.5) doit être proche de la moyenne des 4 pixels
    std::vector<float> data = {
        2.0f, 0.0f, 0.0f,   // (0,0)
        0.0f, 2.0f, 0.0f,   // (1,0)
        0.0f, 0.0f, 2.0f,   // (0,1)
        2.0f, 2.0f, 2.0f,   // (1,1)
    };
    EnvMap e(std::move(data), 2, 2);
    Vec3d c = e.sample(0.5, 0.5);
    // Moyenne : r=(2+0+0+2)/4=1, g=(0+2+0+2)/4=1, b=(0+0+2+2)/4=1
    REQUIRE(c.x == Approx(1.0).epsilon(0.1));
    REQUIRE(c.y == Approx(1.0).epsilon(0.1));
    REQUIRE(c.z == Approx(1.0).epsilon(0.1));
}

TEST_CASE("EnvMap / sample UV wrapping — u=1.0 ≈ u=0.0") {
    EnvMap e = make_uniform_map(8, 4, 3.0f);
    Vec3d c0 = e.sample(0.0,  0.5);
    Vec3d c1 = e.sample(0.99, 0.5);  // bord droit ≈ bord gauche sur uniforme
    REQUIRE(c0.x == Approx(c1.x).epsilon(0.01));
}

// ═══════════════════════════════════════════════════════════════════════════════
// EnvMap — dir_to_uv / uv_to_dir
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("EnvMap / dir_to_uv — (1,0,0) → u=0.5, v=0.5") {
    auto [u, v] = EnvMap::dir_to_uv(Vec3d(1, 0, 0));
    REQUIRE(u == Approx(0.5).epsilon(1e-4));
    REQUIRE(v == Approx(0.5).epsilon(1e-4));
}

TEST_CASE("EnvMap / dir_to_uv — pôle haut (0,1,0) → v=0") {
    auto [u, v] = EnvMap::dir_to_uv(Vec3d(0, 1, 0));
    REQUIRE(v == Approx(0.0).margin(1e-4));
}

TEST_CASE("EnvMap / dir_to_uv — pôle bas (0,-1,0) → v=1") {
    auto [u, v] = EnvMap::dir_to_uv(Vec3d(0, -1, 0));
    REQUIRE(v == Approx(1.0).margin(1e-4));
}

TEST_CASE("EnvMap / dir_to_uv — (0,0,1) → u=0.75, v=0.5") {
    // phi = atan2(1,0) = π/2 → u = π/2/(2π) + 0.5 = 0.75
    auto [u, v] = EnvMap::dir_to_uv(Vec3d(0, 0, 1));
    REQUIRE(u == Approx(0.75).epsilon(1e-4));
    REQUIRE(v == Approx(0.5 ).epsilon(1e-4));
}

TEST_CASE("EnvMap / dir_to_uv — (0,0,-1) → u=0.25, v=0.5") {
    // phi = atan2(-1,0) = -π/2 → u = -π/2/(2π) + 0.5 = 0.25
    auto [u, v] = EnvMap::dir_to_uv(Vec3d(0, 0, -1));
    REQUIRE(u == Approx(0.25).epsilon(1e-4));
    REQUIRE(v == Approx(0.5 ).epsilon(1e-4));
}

TEST_CASE("EnvMap / uv_to_dir retourne un vecteur unitaire") {
    std::vector<std::pair<double,double>> uvs = {
        {0.0, 0.0}, {0.5, 0.5}, {0.25, 0.75}, {0.9, 0.1}, {0.1, 0.9}
    };
    for (auto [u, v] : uvs) {
        Vec3d d = EnvMap::uv_to_dir(u, v);
        REQUIRE(d.length() == Approx(1.0).epsilon(1e-6));
    }
}

TEST_CASE("EnvMap / round-trip dir → uv → dir") {
    std::vector<Vec3d> dirs = {
        Vec3d(1, 0, 0), Vec3d(0, 0, 1), Vec3d(0, 0, -1),
        Vec3d(1, 1, 0).normalized(), Vec3d(1, 0, 1).normalized(),
        Vec3d(0.5, -0.3, 0.8).normalized(),
        Vec3d(-0.7, 0.2, -0.4).normalized(),
    };
    for (auto& dir : dirs) {
        auto [u, v]  = EnvMap::dir_to_uv(dir);
        Vec3d dir2   = EnvMap::uv_to_dir(u, v);
        REQUIRE(dir2.x == Approx(dir.x).margin(1e-5));
        REQUIRE(dir2.y == Approx(dir.y).margin(1e-5));
        REQUIRE(dir2.z == Approx(dir.z).margin(1e-5));
    }
}

TEST_CASE("EnvMap / round-trip uv → dir → uv") {
    std::vector<std::pair<double,double>> uvs = {
        {0.1, 0.1}, {0.5, 0.5}, {0.3, 0.7}, {0.8, 0.2}, {0.9, 0.9}
    };
    for (auto [u, v] : uvs) {
        Vec3d dir    = EnvMap::uv_to_dir(u, v);
        auto [u2,v2] = EnvMap::dir_to_uv(dir);
        REQUIRE(u2 == Approx(u).margin(1e-5));
        REQUIRE(v2 == Approx(v).margin(1e-5));
    }
}

TEST_CASE("EnvMap / dir_to_uv — valeurs dans [0, 1]") {
    std::mt19937 gen(123);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (int i = 0; i < 100; ++i) {
        Vec3d d;
        do { d = Vec3d(dist(gen), dist(gen), dist(gen)); } while (d.length() < 0.01);
        d = d.normalized();
        auto [u, v] = EnvMap::dir_to_uv(d);
        REQUIRE(u >= 0.0);
        REQUIRE(u <= 1.0);
        REQUIRE(v >= 0.0);
        REQUIRE(v <= 1.0);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// EnvMap::background
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("EnvMap / background cohérent avec sample") {
    EnvMap e = make_uniform_map(16, 8, 1.5f);
    Vec3d dir = Vec3d(1, 0, 0).normalized();
    auto [u, v] = EnvMap::dir_to_uv(dir);
    Vec3d c_direct    = e.sample(u, v);
    Vec3d c_background = e.background(dir);
    REQUIRE(c_direct.x    == Approx(c_background.x).epsilon(0.01));
    REQUIRE(c_direct.y    == Approx(c_background.y).epsilon(0.01));
    REQUIRE(c_direct.z    == Approx(c_background.z).epsilon(0.01));
}

TEST_CASE("EnvMap / background retourne des valeurs finies") {
    EnvMap e = make_uniform_map(8, 4, 2.0f);
    std::vector<Vec3d> dirs = {
        Vec3d(1,0,0), Vec3d(0,1,0), Vec3d(0,-1,0), Vec3d(0,0,1),
        Vec3d(1,1,1).normalized(), Vec3d(-1,-1,-1).normalized()
    };
    for (auto& d : dirs) {
        Vec3d c = e.background(d);
        REQUIRE(std::isfinite(c.x));
        REQUIRE(std::isfinite(c.y));
        REQUIRE(std::isfinite(c.z));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// EnvMapSampler
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("EnvMapSampler / directions échantillonnées sont unitaires") {
    EnvMap e = make_uniform_map(16, 8, 1.0f);
    EnvMapSampler sampler(e);

    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < 100; ++i) {
        EnvSample s = sampler.sample(dist(gen), dist(gen));
        REQUIRE(s.direction.length() == Approx(1.0).epsilon(1e-5));
    }
}

TEST_CASE("EnvMapSampler / PDF > 0 pour toutes les directions") {
    EnvMap e = make_uniform_map(16, 8, 1.0f);
    EnvMapSampler sampler(e);

    std::vector<Vec3d> dirs = {
        Vec3d(1,0,0), Vec3d(0,0,1), Vec3d(0,0,-1),
        Vec3d(1,1,1).normalized(), Vec3d(-1,0.5,0.2).normalized()
    };
    for (auto& d : dirs) {
        REQUIRE(sampler.pdf(d) > 0.0);
    }
}

TEST_CASE("EnvMapSampler / PDF s'intègre à ≈ 1 (Monte Carlo)") {
    // ∫ pdf(ω) dω = 1
    // Estimation : sample N directions uniformément sur la sphère,
    // moyenne(pdf(ω_i) × 4π) ≈ 1
    EnvMap e = make_uniform_map(32, 16, 1.0f);
    EnvMapSampler sampler(e);

    std::mt19937 gen(7);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    const int N = 5000;
    double acc = 0.0;
    for (int i = 0; i < N; ++i) {
        double u1 = dist(gen), u2 = dist(gen);
        double theta = std::acos(1.0 - 2.0 * u1);
        double phi   = 2 * rt::pi * u2;
        Vec3d dir(std::sin(theta)*std::cos(phi), std::cos(theta), std::sin(theta)*std::sin(phi));
        acc += sampler.pdf(dir);
    }
    double integral = acc / N * 4.0 * rt::pi;
    REQUIRE(integral == Approx(1.0).epsilon(0.1));  // tolérance 10 %
}

TEST_CASE("EnvMapSampler / carte uniforme → PDF ≈ 1/(4π) partout") {
    EnvMap e = make_uniform_map(32, 16, 1.0f);
    EnvMapSampler sampler(e);

    double uniform_pdf = 1.0 / (4.0 * rt::pi);
    std::vector<Vec3d> dirs = {
        Vec3d(1, 0, 0), Vec3d(0, 0, 1), Vec3d(0, 0, -1),
        Vec3d(1, 1, 0).normalized(), Vec3d(0, 1, 1).normalized()
    };
    for (auto& d : dirs) {
        REQUIRE(sampler.pdf(d) == Approx(uniform_pdf).epsilon(0.2));
    }
}

TEST_CASE("EnvMapSampler / zone brillante échantillonnée plus souvent") {
    // Image 1×4 : une seule colonne très brillante (index 2), les autres sombres
    // col 0 = 0.1, col 1 = 0.1, col 2 = 100.0, col 3 = 0.1
    const int W = 4, H = 1;
    std::vector<float> data(W * H * 3, 0.1f);
    data[2*3+0] = data[2*3+1] = data[2*3+2] = 100.0f;  // col 2 très brillant
    EnvMap e(std::move(data), W, H);
    EnvMapSampler sampler(e);

    std::mt19937 gen(99);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    const int N = 1000;
    int bright_count = 0;
    for (int i = 0; i < N; ++i) {
        EnvSample s = sampler.sample(dist(gen), dist(gen));
        // La colonne 2 correspond à u ∈ [0.5, 0.75)
        auto [u, v] = EnvMap::dir_to_uv(s.direction);
        if (u >= 0.5 && u < 0.75) ++bright_count;
    }
    // 100 / (3×0.1 + 100) ≈ 97 % des échantillons
    REQUIRE(bright_count > 800);
}

TEST_CASE("EnvMapSampler / radiance dans EnvSample cohérente avec background") {
    EnvMap e = make_uniform_map(16, 8, 2.0f);
    EnvMapSampler sampler(e);

    EnvSample s = sampler.sample(0.3, 0.7);
    Vec3d bg = e.background(s.direction);
    REQUIRE(s.radiance.x == Approx(bg.x).epsilon(0.1));
    REQUIRE(s.radiance.y == Approx(bg.y).epsilon(0.1));
    REQUIRE(s.radiance.z == Approx(bg.z).epsilon(0.1));
}
