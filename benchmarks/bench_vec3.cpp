#include <benchmark/benchmark.h>
#include "vec3.h"

using namespace rt;
// ── Addition ──────────────────────────────────────────────────────────────────
static void BM_Vec3_Add(benchmark::State& state) {
    Vec3d a(1.0, 2.0, 3.0), b(4.0, 5.0, 6.0);
    for (auto _ : state) {
        Vec3d r = a + b;
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_Vec3_Add);

// ── Soustraction ──────────────────────────────────────────────────────────────
static void BM_Vec3_Sub(benchmark::State& state) {
    Vec3d a(1.0, 2.0, 3.0), b(4.0, 5.0, 6.0);
    for (auto _ : state) {
        Vec3d r = a - b;
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_Vec3_Sub);

// ── Multiplication scalaire ───────────────────────────────────────────────────
static void BM_Vec3_Scale(benchmark::State& state) {
    Vec3d v(1.0, 2.0, 3.0);
    for (auto _ : state) {
        Vec3d r = v * 2.5;
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_Vec3_Scale);

// ── Produit scalaire ──────────────────────────────────────────────────────────
static void BM_Vec3_Dot(benchmark::State& state) {
    Vec3d a(1.0, 2.0, 3.0), b(4.0, 5.0, 6.0);
    for (auto _ : state) {
        double r = dot(a, b);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_Vec3_Dot);

// ── Produit vectoriel ─────────────────────────────────────────────────────────
static void BM_Vec3_Cross(benchmark::State& state) {
    Vec3d a(1.0, 2.0, 3.0), b(4.0, 5.0, 6.0);
    for (auto _ : state) {
        Vec3d r = a.cross(b);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_Vec3_Cross);

// ── Normalisation ─────────────────────────────────────────────────────────────
static void BM_Vec3_Normalized(benchmark::State& state) {
    Vec3d v(3.0, 4.0, 0.0);
    for (auto _ : state) {
        Vec3d r = v.normalized();
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(BM_Vec3_Normalized);

// ── Chaîne d'opérations (simulant un calcul réel) ────────────────────────────
// Représente le calcul typique d'une direction diffuse : normale + vecteur aléatoire
static void BM_Vec3_DiffuseDir(benchmark::State& state) {
    Vec3d normal(0, 1, 0);
    Vec3d random_dir(0.3, 0.8, 0.1);
    for (auto _ : state) {
        Vec3d dir = (normal + random_dir).normalized();
        benchmark::DoNotOptimize(dir);
    }
}
BENCHMARK(BM_Vec3_DiffuseDir);
