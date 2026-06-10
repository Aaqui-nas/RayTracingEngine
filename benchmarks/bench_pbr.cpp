#include <benchmark/benchmark.h>
#include <cmath>
#include <random>
#include "materials/brdf.h"
#include "core/vec3.h"

using namespace rt;
using namespace rt::physics;

// ─── helpers ──────────────────────────────────────────────────────────────────

static Vec3d rand_unit(double t) {
    double phi   = t * 2.0 * rt::pi;
    double theta = t * rt::pi * 0.5;
    return Vec3d(std::sin(theta) * std::cos(phi),
                 std::cos(theta),
                 std::sin(theta) * std::sin(phi));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Composantes individuelles
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_D_GGX(benchmark::State& state) {
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.00137, 1.0);
        double NdotH     = 0.3 + t * 0.7;
        double roughness = 0.1 + t * 0.8;
        double d = D_GGX(NdotH, roughness);
        benchmark::DoNotOptimize(d);
    }
}
BENCHMARK(BM_D_GGX);

static void BM_G_Smith(benchmark::State& state) {
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.00137, 1.0);
        double NdotL     = 0.1 + t * 0.9;
        double NdotV     = 0.1 + std::fmod(t * 1.618, 0.9);
        double roughness = 0.05 + t * 0.9;
        double g = G_Smith(NdotL, NdotV, roughness);
        benchmark::DoNotOptimize(g);
    }
}
BENCHMARK(BM_G_Smith);

static void BM_F_Schlick(benchmark::State& state) {
    Vec3d F0(0.04, 0.04, 0.04);
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.00137, 1.0);
        Vec3d F = F_Schlick(t, F0);
        benchmark::DoNotOptimize(F);
    }
}
BENCHMARK(BM_F_Schlick);

static void BM_F0_from_ior(benchmark::State& state) {
    double ior = 1.5;
    for (auto _ : state) {
        double f = F0_from_ior(ior);
        benchmark::DoNotOptimize(f);
        ior = 1.0 + std::fmod(ior - 1.0 + 0.01, 1.5);
    }
}
BENCHMARK(BM_F0_from_ior);

// ═══════════════════════════════════════════════════════════════════════════════
// BRDF complète — différentes configurations de roughness
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_BRDF_CookTorrance(benchmark::State& state) {
    double roughness = state.range(0) / 100.0;  // 0.1, 0.5, 1.0
    Vec3d albedo(0.8, 0.6, 0.3);
    Vec3d N(0, 1, 0);
    Vec3d V(0.3, 0.9, 0.1);
    V = V.normalized();

    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.00137, 1.0);
        Vec3d L = rand_unit(t);
        if (dot(L, N) < 0) L = Vec3d(-L.x, L.y, -L.z);  // flip dans l'hémisphère
        Vec3d f = brdf_cook_torrance(albedo, roughness, 0.5, 1.5, N, V, L);
        benchmark::DoNotOptimize(f);
    }
    state.SetLabel("roughness=" + std::to_string((int)(roughness * 100)) + "%");
}
BENCHMARK(BM_BRDF_CookTorrance)->Arg(10)->Arg(50)->Arg(100);

// ═══════════════════════════════════════════════════════════════════════════════
// Cook-Torrance vs Lambertien — coût d'évaluation comparé
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_BRDF_Lambertian_Reference(benchmark::State& state) {
    Vec3d albedo(0.8, 0.8, 0.8);
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.00137, 1.0);
        Vec3d result = albedo * (1.0 / rt::pi);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BRDF_Lambertian_Reference);

static void BM_BRDF_CookTorrance_Metal(benchmark::State& state) {
    Vec3d albedo(0.9, 0.7, 0.3);
    Vec3d N(0, 1, 0);
    Vec3d V(0.3, 0.9, 0.1);
    V = V.normalized();
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.00137, 1.0);
        Vec3d L = rand_unit(t);
        if (dot(L, N) < 0) L = Vec3d(-L.x, L.y, -L.z);
        Vec3d f = brdf_cook_torrance(albedo, 0.3, 1.0, 1.5, N, V, L);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(BM_BRDF_CookTorrance_Metal);

static void BM_BRDF_CookTorrance_Dielectric(benchmark::State& state) {
    Vec3d albedo(0.8, 0.3, 0.3);
    Vec3d N(0, 1, 0);
    Vec3d V(0.3, 0.9, 0.1);
    V = V.normalized();
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.00137, 1.0);
        Vec3d L = rand_unit(t);
        if (dot(L, N) < 0) L = Vec3d(-L.x, L.y, -L.z);
        Vec3d f = brdf_cook_torrance(albedo, 0.5, 0.0, 1.5, N, V, L);
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(BM_BRDF_CookTorrance_Dielectric);

// ═══════════════════════════════════════════════════════════════════════════════
// Material factory — coût de création
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_CookTorrance_Create(benchmark::State& state) {
    for (auto _ : state) {
        Material m = cook_torrance(Vec3d(255, 190, 40), 0.2, 1.0);
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_CookTorrance_Create);

// ═══════════════════════════════════════════════════════════════════════════════
// Material evaluation — coût de scatter() par rebond
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_CookTorrance_Eval(benchmark::State& state) {
    double roughness = state.range(0) / 100.0;
    Material m = cook_torrance(Vec3d(200, 160, 60), roughness, 1.0);
    Ray incoming(Vec3d(0.5, 5.0, 0.3), Vec3d(-0.1, -1.0, -0.05));

    HitRecord rec;
    rec.point      = Vec3d(0, 0, 0);
    rec.normal     = Vec3d(0, 1, 0);
    rec.front_face = true;
    rec.t          = 1.0;
    rec.u          = 0.5;
    rec.v          = 0.5;
    rec.tangent    = Vec3d(1, 0, 0);

    for (auto _ : state) {
        auto scatter = m(incoming, rec);
        benchmark::DoNotOptimize(scatter);
    }
    state.SetLabel("roughness=" + std::to_string((int)(roughness * 100)) + "%");
}
BENCHMARK(BM_CookTorrance_Eval)->Arg(5)->Arg(30)->Arg(70)->Arg(100);

// ═══════════════════════════════════════════════════════════════════════════════
// Impact de metalness sur le coût d'évaluation
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_CookTorrance_Metalness(benchmark::State& state) {
    double metalness = state.range(0) / 100.0;
    Material m = cook_torrance(Vec3d(200, 160, 60), 0.4, metalness);
    Ray incoming(Vec3d(0.3, 5.0, 0.1), Vec3d(-0.05, -1.0, -0.02));

    HitRecord rec;
    rec.point      = Vec3d(0, 0, 0);
    rec.normal     = Vec3d(0, 1, 0);
    rec.front_face = true;
    rec.t          = 1.0;
    rec.u = rec.v  = 0.5;
    rec.tangent    = Vec3d(1, 0, 0);

    for (auto _ : state) {
        auto scatter = m(incoming, rec);
        benchmark::DoNotOptimize(scatter);
    }
    state.SetLabel("metalness=" + std::to_string((int)(metalness * 100)) + "%");
}
BENCHMARK(BM_CookTorrance_Metalness)->Arg(0)->Arg(50)->Arg(100);

// ═══════════════════════════════════════════════════════════════════════════════
// D + G + F séparément vs. ensemble
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_DFG_Separately(benchmark::State& state) {
    double t = 0.0;
    Vec3d F0(0.04, 0.04, 0.04);
    for (auto _ : state) {
        t = std::fmod(t + 0.00137, 1.0);
        double NdotH = 0.3 + t * 0.7;
        double NdotL = 0.2 + t * 0.8;
        double NdotV = 0.4 + std::fmod(t * 1.618, 0.6);
        double VdotH = 0.5 + t * 0.4;
        double r = 0.1 + t * 0.8;

        double d = D_GGX(NdotH, r);
        double g = G_Smith(NdotL, NdotV, r);
        Vec3d  f = F_Schlick(VdotH, F0);
        Vec3d  result = f * (d * g);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DFG_Separately);
