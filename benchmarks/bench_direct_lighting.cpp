#include <benchmark/benchmark.h>
#include <vector>
#include <cmath>
#include <random>
#include "lights/lights.h"
#include "scene/scene.h"
#include "geometry/sphere.h"
#include "materials/materials.h"
#include "core/vec3.h"

using namespace rt;

// ─── helpers ──────────────────────────────────────────────────────────────────

static Material opaque_mat() {
    return [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
        return Scatter{Vec3d(200,200,200), Ray(Vec3d(0,0,0), Vec3d(0,1,0))};
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
// make_sphere_light — coût de construction
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_SphereLightConstruct(benchmark::State& state) {
    for (auto _ : state) {
        Light l = make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(10, 10, 10));
        benchmark::DoNotOptimize(l);
    }
}
BENCHMARK(BM_SphereLightConstruct);

// ═══════════════════════════════════════════════════════════════════════════════
// make_sphere_light — coût d'un sample
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_SphereLightSample(benchmark::State& state) {
    Light l = make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(10, 10, 10));
    Vec3d from(0, 0, 0);
    double xi = 0.0;
    for (auto _ : state) {
        xi = std::fmod(xi + 0.00137, 1.0);
        double xi2 = std::fmod(xi * 1.618, 1.0);
        auto s = l.sample(from, xi, xi2);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_SphereLightSample);

// ═══════════════════════════════════════════════════════════════════════════════
// make_sphere_light — coût de pdf()
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_SphereLightPdf(benchmark::State& state) {
    Light l = make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(10, 10, 10));
    Vec3d from(0, 0, 0);
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.001, 1.0);
        double theta = t * rt::pi * 0.5;  // directions dans le cône
        Vec3d dir(std::sin(theta), std::cos(theta), 0.0);
        double p = l.pdf(from, dir);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(BM_SphereLightPdf);

// ═══════════════════════════════════════════════════════════════════════════════
// LightList — construction avec N lumières
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_LightListBuild(benchmark::State& state) {
    int n = (int)state.range(0);
    for (auto _ : state) {
        LightList ll;
        for (int i = 0; i < n; ++i)
            ll.add(make_sphere_light(Vec3d(i * 2.0, 5, 0), 0.5, Vec3d(8, 8, 8)));
        benchmark::DoNotOptimize(ll);
    }
}
BENCHMARK(BM_LightListBuild)->Arg(1)->Arg(4)->Arg(16);

// ═══════════════════════════════════════════════════════════════════════════════
// LightList — sample avec N lumières
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_LightListSample(benchmark::State& state) {
    int n = (int)state.range(0);
    LightList ll;
    for (int i = 0; i < n; ++i)
        ll.add(make_sphere_light(Vec3d(i * 2.0, 5, 0), 0.5, Vec3d(8, 8, 8)));

    Vec3d from(0, 0, 0);
    double xi = 0.0;
    for (auto _ : state) {
        xi = std::fmod(xi + 0.00137, 1.0);
        double xi2 = std::fmod(xi * 1.618, 1.0);
        double xi3 = std::fmod(xi * 2.718, 1.0);
        auto s = ll.sample(from, xi, xi2, xi3);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_LightListSample)->Arg(1)->Arg(4)->Arg(16);

// ═══════════════════════════════════════════════════════════════════════════════
// LightList::pdf — évaluation avec N lumières
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_LightListPdf(benchmark::State& state) {
    int n = (int)state.range(0);
    LightList ll;
    for (int i = 0; i < n; ++i)
        ll.add(make_sphere_light(Vec3d(i * 2.0, 5, 0), 0.5, Vec3d(8, 8, 8)));

    Vec3d from(0, 0, 0);
    double t = 0.0;
    for (auto _ : state) {
        t = std::fmod(t + 0.001, 1.0);
        Vec3d dir(0, 1, std::sin(t * rt::pi));
        dir = dir.normalized();
        double p = ll.pdf(from, dir);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(BM_LightListPdf)->Arg(1)->Arg(4)->Arg(16);

// ═══════════════════════════════════════════════════════════════════════════════
// Scene::occluded — sans occludeur
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_OccludedEmpty(benchmark::State& state) {
    Scene scene;
    Vec3d from(0, 0, 0);
    Vec3d to(0, 5, 0);
    for (auto _ : state) {
        bool b = scene.occluded(from, to);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(BM_OccludedEmpty);

// ═══════════════════════════════════════════════════════════════════════════════
// Scene::occluded — avec N sphères
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_OccludedWithSpheres(benchmark::State& state) {
    int n = (int)state.range(0);
    Scene scene;
    for (int i = 0; i < n; ++i)
        scene.add(std::make_shared<Sphere>(Vec3d(i * 3.0 + 10, 0, 0), 0.5, opaque_mat()));

    Vec3d from(0, 0, 0);
    Vec3d to(0, 5, 0);  // les sphères sont à x>10, donc non bloquantes
    for (auto _ : state) {
        bool b = scene.occluded(from, to);
        benchmark::DoNotOptimize(b);
    }
    state.SetLabel(std::to_string(n) + " spheres, unblocked");
}
BENCHMARK(BM_OccludedWithSpheres)->Arg(1)->Arg(8)->Arg(32);

// ═══════════════════════════════════════════════════════════════════════════════
// mis_weight — coût de l'heuristique
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_MisWeight(benchmark::State& state) {
    double a = 0.1, b = 0.9;
    for (auto _ : state) {
        double w = mis_weight(a, b);
        benchmark::DoNotOptimize(w);
        a = std::fmod(a + 0.00137, 1.0) + 0.001;
        b = 1.0 - a + 0.001;
    }
}
BENCHMARK(BM_MisWeight);

// ─── comparaison : NEE vs sampling naïf ───────────────────────────────────────
// Ces benchmarks mesurent le coût additionnel du shadow ray par sample.

static void BM_ShadowRayCostBlocked(benchmark::State& state) {
    // Un occludeur direct sur le trajet
    Scene scene;
    scene.add(std::make_shared<Sphere>(Vec3d(0, 2.5, 0), 1.0, opaque_mat()));
    Vec3d from(0, 0.5, 0);
    Vec3d to(0, 4.5, 0);
    for (auto _ : state) {
        bool b = scene.occluded(from, to);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(BM_ShadowRayCostBlocked);

static void BM_ShadowRayCostFree(benchmark::State& state) {
    // Un occludeur hors du trajet
    Scene scene;
    scene.add(std::make_shared<Sphere>(Vec3d(5, 0, 0), 1.0, opaque_mat()));
    Vec3d from(0, 0, 0);
    Vec3d to(0, 5, 0);
    for (auto _ : state) {
        bool b = scene.occluded(from, to);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(BM_ShadowRayCostFree);
