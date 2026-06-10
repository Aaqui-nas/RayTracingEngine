#include <benchmark/benchmark.h>
#include <vector>
#include <cmath>
#include "materials/env_map.h"
#include "core/vec3.h"

using namespace rt;

// ── Helpers ───────────────────────────────────────────────────────────────────

static EnvMap make_uniform_map(int w, int h, float val = 1.0f) {
    std::vector<float> data(w * h * 3, val);
    return EnvMap(std::move(data), w, h);
}

static EnvMap make_gradient_map(int w, int h) {
    std::vector<float> data(w * h * 3);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 3;
            data[idx+0] = (float)x / w;
            data[idx+1] = (float)y / h;
            data[idx+2] = 0.5f;
        }
    }
    return EnvMap(std::move(data), w, h);
}

// ═══════════════════════════════════════════════════════════════════════════════
// rgbe_to_float
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_rgbe_to_float(benchmark::State& state) {
    for (auto _ : state) {
        Vec3d v = rgbe_to_float(128, 200, 77, 143);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_rgbe_to_float);

// ═══════════════════════════════════════════════════════════════════════════════
// EnvMap::dir_to_uv / uv_to_dir
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_dir_to_uv(benchmark::State& state) {
    Vec3d dir(0.6, 0.3, -0.7);
    dir = dir.normalized();
    for (auto _ : state) {
        auto [u, v] = EnvMap::dir_to_uv(dir);
        benchmark::DoNotOptimize(u);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_dir_to_uv);

static void BM_uv_to_dir(benchmark::State& state) {
    double u = 0.37, v = 0.62;
    for (auto _ : state) {
        Vec3d d = EnvMap::uv_to_dir(u, v);
        benchmark::DoNotOptimize(d);
        u = std::fmod(u + 0.001, 1.0);
    }
}
BENCHMARK(BM_uv_to_dir);

// ═══════════════════════════════════════════════════════════════════════════════
// EnvMap::sample — différentes résolutions
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_EnvMap_Sample(benchmark::State& state) {
    int sz = (int)state.range(0);
    EnvMap e = make_gradient_map(sz * 2, sz);
    double u = 0.0;
    for (auto _ : state) {
        u = std::fmod(u + 0.001, 1.0);
        Vec3d c = e.sample(u, 1.0 - u);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_EnvMap_Sample)->Arg(64)->Arg(256)->Arg(512)->Arg(1024);

// ═══════════════════════════════════════════════════════════════════════════════
// EnvMap::background — lookup complet avec conversion direction
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_EnvMap_Background(benchmark::State& state) {
    int sz = (int)state.range(0);
    EnvMap e = make_gradient_map(sz * 2, sz);
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.001, 1.0);
        double theta = t * rt::pi;
        double phi   = t * 2.0 * rt::pi;
        Vec3d dir(std::sin(theta)*std::cos(phi), std::cos(theta), std::sin(theta)*std::sin(phi));
        Vec3d c = e.background(dir);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_EnvMap_Background)->Arg(64)->Arg(256)->Arg(512)->Arg(1024);

// ── Comparaison dégradé de ciel codé en dur vs env map ───────────────────────

static void BM_Sky_Gradient_Hardcoded(benchmark::State& state) {
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.001, 1.0);
        double theta = t * rt::pi;
        Vec3d dir(0, std::cos(theta), std::sin(theta));
        dir = dir.normalized();
        double y   = (dir.y + 1) / 2;
        Vec3d sky  = (1-y) * Vec3d(255,255,255) + y * Vec3d(128,178,255);
        benchmark::DoNotOptimize(sky);
    }
}
BENCHMARK(BM_Sky_Gradient_Hardcoded);

static void BM_EnvMap_Background_256(benchmark::State& state) {
    EnvMap e = make_gradient_map(512, 256);
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.001, 1.0);
        double theta = t * rt::pi;
        Vec3d dir(0, std::cos(theta), std::sin(theta));
        dir = dir.normalized();
        Vec3d c = e.background(dir);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_EnvMap_Background_256);

// ═══════════════════════════════════════════════════════════════════════════════
// EnvMapSampler::sample — importance sampling
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_EnvMapSampler_Build(benchmark::State& state) {
    int sz = (int)state.range(0);
    EnvMap e = make_gradient_map(sz * 2, sz);
    for (auto _ : state) {
        EnvMapSampler sampler(e);
        benchmark::DoNotOptimize(sampler);
    }
}
BENCHMARK(BM_EnvMapSampler_Build)->Arg(64)->Arg(256)->Arg(512);

static void BM_EnvMapSampler_Sample(benchmark::State& state) {
    int sz = (int)state.range(0);
    EnvMap e = make_gradient_map(sz * 2, sz);
    EnvMapSampler sampler(e);
    double xi = 0.0;
    for (auto _ : state) {
        xi = std::fmod(xi + 0.00137, 1.0);
        double xi2 = std::fmod(xi * 1.618, 1.0);
        EnvSample s = sampler.sample(xi, xi2);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_EnvMapSampler_Sample)->Arg(64)->Arg(256)->Arg(512)->Arg(1024);

static void BM_EnvMapSampler_Pdf(benchmark::State& state) {
    int sz = (int)state.range(0);
    EnvMap e = make_gradient_map(sz * 2, sz);
    EnvMapSampler sampler(e);
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.001, 1.0);
        double theta = t * rt::pi;
        double phi   = t * 2.0 * rt::pi;
        Vec3d dir(std::sin(theta)*std::cos(phi), std::cos(theta), std::sin(theta)*std::sin(phi));
        double p = sampler.pdf(dir);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(BM_EnvMapSampler_Pdf)->Arg(64)->Arg(256)->Arg(512);
