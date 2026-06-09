#include <benchmark/benchmark.h>
#include "core/mat3.h"
#include "geometry/sphere.h"
#include "materials/texture.h"
#include "materials/materials.h"

using namespace rt;

// Matériau lambertien minimal — référence sans normal mapping.
static Material base_lambert = [](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
    Vec3d dir = rec.normal + Vec3d(0.5, 0.3, 0.2);  // direction diffuse approximée
    return Scatter{Vec3d(200, 200, 200), Ray(rec.point, dir)};
};

// ── Hit sphère sans normal map (référence) ───────────────────────────────────

static void BM_Sphere_Hit_NoNormal(benchmark::State& state) {
    Sphere s(Vec3d(0, 0, -2), 0.5, base_lambert);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    for (auto _ : state) {
        auto hit = s.hit(r, 0.001, 1000.0);
        if (hit) benchmark::DoNotOptimize(hit->material(r, *hit));
    }
}
BENCHMARK(BM_Sphere_Hit_NoNormal);

// ── Hit sphère avec normal map (surcoût mesuré) ──────────────────────────────

static void BM_Sphere_Hit_NormalMapped(benchmark::State& state) {
    auto bump = std::make_shared<BumpTexture>(4.0, 1.0);
    auto mat  = normal_mapped(base_lambert, bump);
    Sphere s(Vec3d(0, 0, -2), 0.5, mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    for (auto _ : state) {
        auto hit = s.hit(r, 0.001, 1000.0);
        if (hit) benchmark::DoNotOptimize(hit->material(r, *hit));
    }
}
BENCHMARK(BM_Sphere_Hit_NormalMapped);

// ── BumpTexture::sample seul ─────────────────────────────────────────────────

static void BM_BumpTexture_Sample(benchmark::State& state) {
    BumpTexture bump(4.0, 1.0);
    double x = 0.0;
    for (auto _ : state) {
        x = std::fmod(x + 0.001, 1.0);
        Vec3d n = bump.sample(x, x * 0.7, Vec3d(x, x * 0.5, x * 0.3));
        benchmark::DoNotOptimize(n);
    }
}
BENCHMARK(BM_BumpTexture_Sample);

// ── BumpTexture à différentes strengths ─────────────────────────────────────
// Vérifie que la normalisation finale n'est pas le goulot d'étranglement.

static void BM_BumpTexture_Strength(benchmark::State& state) {
    double strength = (double)state.range(0) / 10.0;
    BumpTexture bump(4.0, strength);
    double x = 0.0;
    for (auto _ : state) {
        x = std::fmod(x + 0.001, 1.0);
        Vec3d n = bump.sample(x, x * 0.7, Vec3d(x, x * 0.5, x * 0.3));
        benchmark::DoNotOptimize(n);
    }
}
BENCHMARK(BM_BumpTexture_Strength)->Arg(0)->Arg(10)->Arg(30)->Arg(100);

// ── Transformation TBN seule ─────────────────────────────────────────────────

static void BM_TBN_Transform(benchmark::State& state) {
    Vec3d T(1, 0, 0), B(0, 1, 0), N(0, 0, 1);
    Mat3 tbn{T, B, N};
    Vec3d ts(0.5, 0.3, 0.8);
    for (auto _ : state) {
        Vec3d w = (tbn * ts).normalized();
        benchmark::DoNotOptimize(w);
    }
}
BENCHMARK(BM_TBN_Transform);
