#include <benchmark/benchmark.h>
#include "scene/scene.h"
#include "geometry/sphere.h"
#include "geometry/plane.h"
#include "geometry/triangle.h"
#include "geometry/mesh.h"
#include "geometry/bvh.h"
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

// ── Triangle::hit ─────────────────────────────────────────────────────────────

static void BM_Triangle_Hit(benchmark::State& state) {
    Triangle tri(Vec3d(0,0,-2), Vec3d(1,0,-2), Vec3d(0,1,-2), null_mat);
    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));
    for (auto _ : state) {
        auto hit = tri.hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_Triangle_Hit);

static void BM_Triangle_Miss(benchmark::State& state) {
    Triangle tri(Vec3d(0,0,-2), Vec3d(1,0,-2), Vec3d(0,1,-2), null_mat);
    Ray r(Vec3d(5, 5, 0), Vec3d(0, 0, -1));  // manque le triangle
    for (auto _ : state) {
        auto hit = tri.hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_Triangle_Miss);

// ── Mesh::hit avec N triangles (O(n)) ────────────────────────────────────────
// Servira de référence brute-force à comparer avec le BVH en TP14

static void BM_Mesh_Hit(benchmark::State& state) {
    const int N = state.range(0);
    Mesh mesh(null_mat);

    // Triangles en grille — seul le premier est touché par le rayon
    for (int i = 0; i < N; i++) {
        double x = (i % 10) * 3.0;
        double z = -(i / 10) * 3.0 - 2.0;
        mesh.add(Triangle(
            Vec3d(x,   0, z), Vec3d(x+1, 0, z), Vec3d(x, 1, z),
            null_mat
        ));
    }

    Ray r(Vec3d(0.2, 0.2, 0), Vec3d(0, 0, -1));

    for (auto _ : state) {
        auto hit = mesh.hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
    state.SetItemsProcessed(state.iterations() * N);
}
BENCHMARK(BM_Mesh_Hit)->Range(10, 10000)->RangeMultiplier(10);

// ── BVH::hit (O(log n)) vs Mesh::hit (O(n)) ──────────────────────────────────
// Comparaison directe après TP14 : le BVH amortit le coût sur de grands datasets.

static void BM_BVH_Hit(benchmark::State& state) {
    const int N = state.range(0);
    std::vector<std::shared_ptr<Shape>> shapes;
    shapes.reserve(N);
    for (int i = 0; i < N; i++) {
        double x = (i % 10) * 3.0;
        double z = -(i / 10) * 3.0 - 2.0;
        shapes.push_back(std::make_shared<Sphere>(Vec3d(x, 0, z), 0.4, null_mat));
    }
    BVHPool pool(N); BVHNode* bvh = pool.build(shapes, 0, N);
    Ray r(Vec3d(0.2, 0, 0), Vec3d(0, 0, -1));

    for (auto _ : state) {
        auto hit = bvh->hit(r, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
    state.SetItemsProcessed(state.iterations() * N);
}
BENCHMARK(BM_BVH_Hit)->Range(10, 10000)->RangeMultiplier(10);
