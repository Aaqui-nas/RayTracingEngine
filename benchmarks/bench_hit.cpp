#include <benchmark/benchmark.h>
#include "scene.h"
#include "sphere.h"
#include "plane.h"
#include <vector>
#include <memory>

using namespace rt;
// Matériau vide — on benchmark l'intersection, pas l'évaluation du matériau
static Material null_mat = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return std::nullopt;
};

// ── Sphere::hit ───────────────────────────────────────────────────────────────

static void BM_Sphere_Hit(benchmark::State& state) {
    Sphere s(Vec3d(0, 0, -2), 0.5, null_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    for (auto _ : state) {
        auto hit = s.hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_Sphere_Hit);

static void BM_Sphere_Miss(benchmark::State& state) {
    Sphere s(Vec3d(10, 0, -2), 0.5, null_mat);
    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));
    for (auto _ : state) {
        auto hit = s.hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_Sphere_Miss);

// ── Plane::hit ────────────────────────────────────────────────────────────────

static void BM_Plane_Hit(benchmark::State& state) {
    Plane p(Vec3d(0, -1, 0), Vec3d(0, 1, 0), null_mat);
    Ray r(Vec3d(0, 0, -2), Vec3d(0, -0.3, -1).normalized());
    for (auto _ : state) {
        auto hit = p.hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_Plane_Hit);

// ── Scene::hit avec N sphères (O(n)) ─────────────────────────────────────────
// Ces benchmarks montrent la dégradation linéaire qui motive le BVH (TP14)

static void BM_Scene_Hit(benchmark::State& state) {
    const int N = state.range(0);
    Scene scene;

    // Sphères réparties en grille — seule la première est touchée par le rayon
    for (int i = 0; i < N; i++) {
        double x = (i % 10) * 2.0;
        double z = -(i / 10) * 2.0 - 3.0;
        scene.add(std::make_shared<Sphere>(Vec3d(x, 0, z), 0.4, null_mat));
    }

    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));  // ne touche que la première sphère

    for (auto _ : state) {
        auto hit = scene.hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
    state.SetItemsProcessed(state.iterations() * N);  // tests d'intersection par seconde
}
// Mesure pour 10, 100, 1000, 10000 sphères — courbe O(n) visible
BENCHMARK(BM_Scene_Hit)->Range(10, 10000)->RangeMultiplier(10);

static void BM_Scene_Miss(benchmark::State& state) {
    const int N = state.range(0);
    Scene scene;

    for (int i = 0; i < N; i++) {
        double x = (i % 10) * 2.0 + 5.0;  // toutes décalées — rayon manque tout
        double z = -(i / 10) * 2.0 - 3.0;
        scene.add(std::make_shared<Sphere>(Vec3d(x, 0, z), 0.4, null_mat));
    }

    Ray r(Vec3d(0, 0, 0), Vec3d(0, 0, -1));  // rayon ne touche rien

    for (auto _ : state) {
        auto hit = scene.hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
    state.SetItemsProcessed(state.iterations() * N);
}
BENCHMARK(BM_Scene_Miss)->Range(10, 10000)->RangeMultiplier(10);

// ── Contexte : après TP14, BM_Scene_Hit sera comparé à BM_BVH_Hit ────────────
// Ce benchmark servira de référence "brute force" pour mesurer le gain du BVH.
// La différence attendue : O(n) → O(log n), soit 100-1000× sur de gros datasets.
