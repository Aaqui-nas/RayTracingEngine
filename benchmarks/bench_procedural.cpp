#include <benchmark/benchmark.h>
#include <vector>

#include "geometry/procedural.h"
#include "geometry/mesh.h"
#include "materials/materials.h"

using namespace rt;

static Material dummy_mat() {
    return [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
        return std::nullopt;
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 2 — Tore : coût de génération selon la résolution
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_ProceduralTorus(benchmark::State& state) {
    int segments = state.range(0);
    for (auto _ : state) {
        Mesh m = generate_torus(1.0, 0.3, segments, segments, dummy_mat());
        benchmark::DoNotOptimize(m);
    }
    state.SetItemsProcessed(state.iterations() * 2 * segments * segments);
    state.SetLabel(std::to_string(2 * segments * segments) + " triangles");
}
BENCHMARK(BM_ProceduralTorus)->Arg(8)->Arg(16)->Arg(32)->Arg(64);

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 3 — Terrain : coût de génération selon la taille de la grille
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_ProceduralTerrain(benchmark::State& state) {
    int N = state.range(0);
    std::vector<std::vector<double>> heights(N, std::vector<double>(N, 0.0));
    // Quelques variations de hauteur
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            heights[i][j] = 0.5 * std::sin(i * 0.3) * std::cos(j * 0.3);

    for (auto _ : state) {
        Mesh m = generate_terrain(heights, 1.0, 1.0, dummy_mat());
        benchmark::DoNotOptimize(m);
    }
    state.SetItemsProcessed(state.iterations() * 2 * (N-1) * (N-1));
    state.SetLabel(std::to_string(2 * (N-1) * (N-1)) + " triangles");
}
BENCHMARK(BM_ProceduralTerrain)->Arg(8)->Arg(16)->Arg(32)->Arg(64)->Arg(128);

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 4 — Sphere mesh : comparaison avec la primitive implicite
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_ProceduralSphereMesh_Gen(benchmark::State& state) {
    int segments = state.range(0);
    for (auto _ : state) {
        Mesh m = generate_sphere_mesh(1.0, segments, segments, dummy_mat());
        benchmark::DoNotOptimize(m);
    }
    state.SetLabel(std::to_string(segments) + "x" + std::to_string(segments));
}
BENCHMARK(BM_ProceduralSphereMesh_Gen)->Arg(8)->Arg(16)->Arg(32);

// ═══════════════════════════════════════════════════════════════════════════════
// Coût du hit sur des maillages de tailles différentes (sans BVH)
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_ProceduralTorus_Hit(benchmark::State& state) {
    int segments = state.range(0);
    Mesh m = generate_torus(1.0, 0.3, segments, segments, dummy_mat());
    Ray ray(Vec3d(2.0, 0.0, 0.0), Vec3d(-1, 0, 0));

    for (auto _ : state) {
        auto hit = m.hit(ray, 0.001, 100.0);
        benchmark::DoNotOptimize(hit);
    }
    state.SetLabel(std::to_string(2 * segments * segments) + " tris");
}
BENCHMARK(BM_ProceduralTorus_Hit)->Arg(8)->Arg(16)->Arg(32)->Arg(64);

static void BM_ProceduralTerrain_Hit(benchmark::State& state) {
    int N = state.range(0);
    std::vector<std::vector<double>> heights(N, std::vector<double>(N, 0.0));
    Mesh m = generate_terrain(heights, 1.0, 1.0, dummy_mat());
    Ray ray(Vec3d(float(N)/2, 5.0, float(N)/2), Vec3d(0, -1, 0));

    for (auto _ : state) {
        auto hit = m.hit(ray, 0.001, 100.0);
        benchmark::DoNotOptimize(hit);
    }
    state.SetLabel(std::to_string(2 * (N-1) * (N-1)) + " tris");
}
BENCHMARK(BM_ProceduralTerrain_Hit)->Arg(8)->Arg(16)->Arg(32);

// ═══════════════════════════════════════════════════════════════════════════════
// Bonus — apply_noise sur un terrain
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_ApplyNoise(benchmark::State& state) {
    int N = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<std::vector<double>> heights(N, std::vector<double>(N, 0.0));
        state.ResumeTiming();

        apply_noise(heights, 0.1, 1.0);
        benchmark::DoNotOptimize(heights);
    }
    state.SetLabel(std::to_string(N) + "x" + std::to_string(N));
}
BENCHMARK(BM_ApplyNoise)->Arg(16)->Arg(64)->Arg(256);