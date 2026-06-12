#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <numeric>
#include <cmath>
#include "core/post_process.h"

using namespace rt;
using Catch::Approx;

// ── Helpers ───────────────────────────────────────────────────────────────────

static ImageBuffer solid(int w, int h, double r, double g, double b) {
    ImageBuffer buf(w, h);
    for (auto& p : buf.pixels) p = {r, g, b};
    return buf;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PostPipeline
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PostPipeline / passes appliquées dans l'ordre", "[postprocess]") {
    std::vector<int> order;
    PostPipeline pipeline;
    pipeline
        .add([&order](ImageBuffer&){ order.push_back(1); })
        .add([&order](ImageBuffer&){ order.push_back(2); })
        .add([&order](ImageBuffer&){ order.push_back(3); });

    ImageBuffer buf(1, 1);
    pipeline.apply(buf);

    REQUIRE(order == std::vector<int>{1, 2, 3});
}

TEST_CASE("PostPipeline / pipeline vide ne modifie pas le buffer", "[postprocess]") {
    PostPipeline pipeline;
    auto buf = solid(4, 4, 0.5, 0.5, 0.5);
    pipeline.apply(buf);
    REQUIRE(buf.at(0, 0).r == Approx(0.5));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tone mapping Reinhard
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Reinhard / f(0) = 0", "[postprocess]") {
    auto buf = solid(1, 1, 0.0, 0.0, 0.0);
    PostPipeline().add(tone_map_reinhard()).apply(buf);
    REQUIRE(buf.at(0,0).r == Approx(0.0));
}

TEST_CASE("Reinhard / f(1) = 0.5", "[postprocess]") {
    auto buf = solid(1, 1, 1.0, 1.0, 1.0);
    PostPipeline().add(tone_map_reinhard()).apply(buf);
    REQUIRE(buf.at(0,0).r == Approx(0.5).epsilon(0.01));
}

TEST_CASE("Reinhard / comprime les valeurs > 1 sans saturer brutalement", "[postprocess]") {
    auto buf = solid(1, 1, 10.0, 10.0, 10.0);
    PostPipeline().add(tone_map_reinhard()).apply(buf);
    REQUIRE(buf.at(0,0).r > 0.9);
    REQUIRE(buf.at(0,0).r < 1.0);
}

TEST_CASE("Reinhard / monotone croissant", "[postprocess]") {
    double prev = -1;
    for (double v : {0.0, 0.5, 1.0, 2.0, 5.0, 10.0}) {
        auto buf = solid(1, 1, v, v, v);
        PostPipeline().add(tone_map_reinhard()).apply(buf);
        REQUIRE(buf.at(0,0).r > prev);
        prev = buf.at(0,0).r;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tone mapping ACES
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("ACES / output dans [0, 1]", "[postprocess]") {
    for (double v : {0.0, 0.5, 1.0, 2.0, 10.0, 100.0}) {
        auto buf = solid(1, 1, v, v, v);
        PostPipeline().add(tone_map_aces()).apply(buf);
        REQUIRE(buf.at(0,0).r >= 0.0);
        REQUIRE(buf.at(0,0).r <= 1.0);
    }
}

TEST_CASE("ACES / f(0) = 0", "[postprocess]") {
    auto buf = solid(1, 1, 0.0, 0.0, 0.0);
    PostPipeline().add(tone_map_aces()).apply(buf);
    REQUIRE(buf.at(0,0).r == Approx(0.0).margin(0.01));
}

TEST_CASE("ACES / plus contrasté que Reinhard pour valeur > 1", "[postprocess]") {
    auto buf_aces     = solid(1, 1, 2.0, 2.0, 2.0);
    auto buf_reinhard = solid(1, 1, 2.0, 2.0, 2.0);
    PostPipeline().add(tone_map_aces()).apply(buf_aces);
    PostPipeline().add(tone_map_reinhard()).apply(buf_reinhard);
    // ACES donne un résultat plus lumineux que Reinhard pour les hautes lumières
    REQUIRE(buf_aces.at(0,0).r > buf_reinhard.at(0,0).r);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Gamma correction
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Gamma / f(0) = 0, f(1) = 1", "[postprocess]") {
    auto buf0 = solid(1, 1, 0.0, 0.0, 0.0);
    auto buf1 = solid(1, 1, 1.0, 1.0, 1.0);
    PostPipeline().add(gamma_correct(2.2)).apply(buf0);
    PostPipeline().add(gamma_correct(2.2)).apply(buf1);
    REQUIRE(buf0.at(0,0).r == Approx(0.0).margin(0.001));
    REQUIRE(buf1.at(0,0).r == Approx(1.0).epsilon(0.001));
}

TEST_CASE("Gamma / gris moyen est éclairci (gamma 2.2)", "[postprocess]") {
    auto buf = solid(1, 1, 0.5, 0.5, 0.5);
    PostPipeline().add(gamma_correct(2.2)).apply(buf);
    // pow(0.5, 1/2.2) ≈ 0.73 > 0.5
    REQUIRE(buf.at(0,0).r == Approx(std::pow(0.5, 1.0/2.2)).epsilon(0.001));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Kernel gaussien
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Kernel gaussien / somme = 1 (normalisé)", "[postprocess]") {
    for (int r : {1, 2, 3, 5}) {
        auto k = make_gaussian_kernel(r, 1.0);
        double sum = std::accumulate(k.begin(), k.end(), 0.0);
        REQUIRE(sum == Approx(1.0).epsilon(0.001));
    }
}

TEST_CASE("Kernel gaussien / taille = (2r+1)²", "[postprocess]") {
    for (int r : {1, 2, 3}) {
        auto k = make_gaussian_kernel(r, 1.0);
        REQUIRE(k.size() == (size_t)(2*r+1) * (2*r+1));
    }
}

TEST_CASE("Kernel gaussien / symétrique (centre est le max)", "[postprocess]") {
    int r = 3;
    auto k = make_gaussian_kernel(r, 1.0);
    int center = r * (2*r+1) + r;
    for (int i = 0; i < (int)k.size(); i++)
        REQUIRE(k[center] >= k[i]);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Convolution
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("convolve / kernel identité retourne une copie exacte", "[postprocess]") {
    auto src = solid(8, 8, 0.3, 0.5, 0.7);
    src.at(3, 3) = {2.0, 1.5, 0.1};

    // Kernel identité : 1 au centre, 0 partout
    std::vector<double> identity(9, 0.0);
    identity[4] = 1.0;

    auto dst = convolve(src, identity, 1);

    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++) {
            REQUIRE(dst.at(x,y).r == Approx(src.at(x,y).r).margin(1e-9));
            REQUIRE(dst.at(x,y).g == Approx(src.at(x,y).g).margin(1e-9));
            REQUIRE(dst.at(x,y).b == Approx(src.at(x,y).b).margin(1e-9));
        }
}

TEST_CASE("convolve / kernel uniforme = moyenne des voisins", "[postprocess]") {
    // Image 3x3 avec valeurs distinctes
    ImageBuffer src(3, 3);
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            src.at(x, y) = {(double)(y*3+x), 0, 0};

    // Kernel 3x3 uniforme : chaque coefficient = 1/9
    std::vector<double> uniform(9, 1.0 / 9.0);
    auto dst = convolve(src, uniform, 1);

    // Le centre (1,1) est la moyenne de tous les pixels
    double expected = (0+1+2+3+4+5+6+7+8) / 9.0;
    REQUIRE(dst.at(1, 1).r == Approx(expected).epsilon(0.01));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Bloom
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Bloom / pixel sombre (<threshold) inchangé", "[postprocess]") {
    auto buf = solid(16, 16, 0.5, 0.5, 0.5);
    double before = buf.at(8, 8).r;
    PostPipeline().add(bloom_pass(1.5, 3, 0.3)).apply(buf);
    REQUIRE(buf.at(8, 8).r == Approx(before).margin(0.01));
}

TEST_CASE("Bloom / pixel brillant se propage aux voisins", "[postprocess]") {
    auto buf = solid(32, 32, 0.0, 0.0, 0.0);
    buf.at(16, 16) = {5.0, 5.0, 5.0};   // un seul pixel très brillant

    PostPipeline().add(bloom_pass(1.5, 5, 1.0)).apply(buf);

    // Les voisins immédiats doivent avoir reçu de l'éclat
    REQUIRE(buf.at(17, 16).r > 0.0);
    REQUIRE(buf.at(15, 16).r > 0.0);
    REQUIRE(buf.at(16, 17).r > 0.0);
}
