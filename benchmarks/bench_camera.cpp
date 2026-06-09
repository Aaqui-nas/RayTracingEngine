#include <benchmark/benchmark.h>
#include "camera.h"

using namespace rt;

// ── Camera::get_ray sans DOF ─────────────────────────────────────────────────
// Appelé width × height × samples fois par image — doit être quasi-gratuit.

static void BM_Camera_GetRay_NoDOF(benchmark::State& state) {
    Camera cam(Vec3d(0, 1, 4), Vec3d(0, 0.5, 0), Vec3d(0, 1, 0),
               60.0, 16.0/9.0, 0.0, 4.0);
    double u = 0.0;
    for (auto _ : state) {
        u += 1.0 / state.max_iterations;
        Ray r = cam.get_ray(u, 0.5);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_Camera_GetRay_NoDOF);

// ── Camera::get_ray avec DOF ─────────────────────────────────────────────────
// Surcoût lié à l'échantillonnage aléatoire du disque de l'objectif.

static void BM_Camera_GetRay_WithDOF(benchmark::State& state) {
    Camera cam(Vec3d(0, 1, 4), Vec3d(0, 0.5, 0), Vec3d(0, 1, 0),
               60.0, 16.0/9.0, 0.1, 4.0);
    double u = 0.0;
    for (auto _ : state) {
        u += 1.0 / state.max_iterations;
        Ray r = cam.get_ray(u, 0.5);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_Camera_GetRay_WithDOF);
