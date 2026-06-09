#include <benchmark/benchmark.h>
#include "materials/texture.h"
#include <vector>
#include <cstdint>

using namespace rt;

// ── SolidColor::sample ───────────────────────────────────────────────────────

static void BM_SolidColor_Sample(benchmark::State& state) {
    SolidColor tex(Vec3d(200, 100, 50));
    for (auto _ : state) {
        Vec3d c = tex.sample(0.5, 0.5, Vec3d(0,0,0));
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_SolidColor_Sample);

// ── CheckerTexture::sample ───────────────────────────────────────────────────

static void BM_CheckerTexture_Sample(benchmark::State& state) {
    CheckerTexture tex(Vec3d(255,255,255), Vec3d(0,0,0), 8.0);
    double u = 0.0;
    for (auto _ : state) {
        u = std::fmod(u + 0.001, 1.0);
        Vec3d c = tex.sample(u, u, Vec3d(0,0,0));
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_CheckerTexture_Sample);

// ── ImageTexture::sample (512×512) ───────────────────────────────────────────
// Simule le coût d'un accès mémoire dans une grande texture.

static void BM_ImageTexture_Sample(benchmark::State& state) {
    const int W = 512, H = 512;
    std::vector<uint8_t> data(W * H * 3);
    for (int i = 0; i < W * H * 3; i++) data[i] = (uint8_t)(i % 256);
    ImageTexture tex(data, W, H);

    double u = 0.0;
    for (auto _ : state) {
        u = std::fmod(u + 0.001, 1.0);
        Vec3d c = tex.sample(u, 1.0 - u, Vec3d(0,0,0));
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_ImageTexture_Sample);

// ── PerlinNoise::noise ────────────────────────────────────────────────────────

static void BM_PerlinNoise_1Sample(benchmark::State& state) {
    PerlinNoise pn;
    double x = 0.0;
    for (auto _ : state) {
        x += 0.01;
        double n = pn.noise(Vec3d(x, x * 0.7, x * 0.3));
        benchmark::DoNotOptimize(n);
    }
}
BENCHMARK(BM_PerlinNoise_1Sample);

// ── PerlinNoise::turbulence (N octaves) ──────────────────────────────────────
// Coût proportionnel au nombre d'octaves — base pour choisir le bon compromis.

static void BM_PerlinNoise_Turbulence(benchmark::State& state) {
    const int octaves = state.range(0);
    PerlinNoise pn;
    double x = 0.0;
    for (auto _ : state) {
        x += 0.01;
        double t = pn.turbulence(Vec3d(x, x * 0.7, x * 0.3), octaves);
        benchmark::DoNotOptimize(t);
    }
}
BENCHMARK(BM_PerlinNoise_Turbulence)->Arg(1)->Arg(4)->Arg(7)->Arg(12);
