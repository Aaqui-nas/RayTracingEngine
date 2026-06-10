#include <benchmark/benchmark.h>
#include "core/vec3.h"
#include "core/simd.h"
#include <vector>

using namespace rt;

static constexpr int N = 10000;

static void BM_DotNScalar(benchmark::State& state) {
    std::vector<Vec3d> a(N), b(N);
    for (int i = 0; i < N; i++) {
        a[i] = Vec3d(i * 0.1, i * 0.2, i * 0.3);
        b[i] = Vec3d(i * 0.3, i * 0.1, i * 0.2);
    }

    for (auto _ : state) {
        double sum = 0.0;
        for (int i = 0; i < N; i++)
            sum += dot(a[i], b[i]);
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_DotNScalar);

static void BM_DotNAVX(benchmark::State& state) {
    std::vector<Vec3x4> a(N / 4), b(N / 4);
    for (int i = 0; i < N / 4; i++) {
        for (int k = 0; k < 4; k++) {
            int idx = i * 4 + k;
            a[i].x[k] = idx * 0.1;
            a[i].y[k] = idx * 0.2;
            a[i].z[k] = idx * 0.3;
            b[i].x[k] = idx * 0.3;
            b[i].y[k] = idx * 0.1;
            b[i].z[k] = idx * 0.2;
        }
    }

    alignas(32) double r[4];
    for (auto _ : state) {
        double sum = 0.0;
        for (int i = 0; i < N / 4; i++) {
            dot4(a[i], b[i], r);
            sum += r[0] + r[1] + r[2] + r[3];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_DotNAVX);


static void BM_LengthSqNScalar(benchmark::State& state) {
    std::vector<Vec3d> v(N);
    for (int i = 0; i < N; i++)
        v[i] = Vec3d(i * 0.1, i * 0.2, i * 0.3);

    for (auto _ : state) {
        double sum = 0.0;
        for (int i = 0; i < N; i++)
            sum += v[i].length_squared();
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_LengthSqNScalar);

static void BM_LengthSqNAVX(benchmark::State& state) {
    std::vector<Vec3x4> v(N / 4);
    for (int i = 0; i < N / 4; i++) {
        for (int k = 0; k < 4; k++) {
            int idx = i * 4 + k;
            v[i].x[k] = idx * 0.1;
            v[i].y[k] = idx * 0.2;
            v[i].z[k] = idx * 0.3;
        }
    }

    alignas(32) double r[4];
    for (auto _ : state) {
        double sum = 0.0;
        for (int i = 0; i < N / 4; i++) {
            length_sq4(v[i], r);
            sum += r[0] + r[1] + r[2] + r[3];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_LengthSqNAVX);


static void BM_NormalizeNScalar(benchmark::State& state) {
    std::vector<Vec3d> v(N);
    for (int i = 0; i < N; i++)
        v[i] = Vec3d(i * 0.1, i * 0.2, i * 0.3);

    for (auto _ : state) {
        double sum = 0.0;
        for (int i = 0; i < N; i++) {
            Vec3d n = v[i].normalized();
            sum += n.x;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_NormalizeNScalar);


static void BM_NormalizeNAVX(benchmark::State& state) {
    std::vector<Vec3x4> v(N / 4);
    for (int i = 0; i < N / 4; i++) {
        for (int k = 0; k < 4; k++) {
            int idx = i * 4 + k;
            v[i].x[k] = idx * 0.1;
            v[i].y[k] = idx * 0.2;
            v[i].z[k] = idx * 0.3;
        }
    }

    for (auto _ : state) {
        double sum = 0.0;
        for (int i = 0; i < N / 4; i++) {
            Vec3x4 n = normalize4(v[i]);
            sum += n.x[0];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_NormalizeNAVX);

// ── Scalaire : N intersections rayon/sphère une par une ──────────────────────

static void BM_HitSphereNScalar(benchmark::State& state) {
    Sphere sphere(Vec3d(0, 0, -1), 0.5, nullptr);

    std::vector<Ray> rays;
    rays.reserve(N);
    for (int i = 0; i < N; i++) {
        double u = (i % 100) / 100.0;
        rays.emplace_back(Vec3d(u - 0.5, 0, 0), Vec3d(0, 0, -1));
    }

    for (auto _ : state) {
        int hits = 0;
        for (int i = 0; i < N; i++) {
            auto rec = sphere.hit(rays[i], 0.001, 1e30);
            if (rec) hits++;
        }
        benchmark::DoNotOptimize(hits);
    }
}
BENCHMARK(BM_HitSphereNScalar);

// ── AVX : N intersections rayon/sphère par blocs de 4 ────────────────────────

static void BM_HitSphereNAVX(benchmark::State& state) {
    Sphere sphere(Vec3d(0, 0, -1), 0.5, nullptr);

    std::vector<Ray4> ray4s(N / 4);
    for (int i = 0; i < N / 4; i++) {
        for (int k = 0; k < 4; k++) {
            int idx = i * 4 + k;
            double u = (idx % 100) / 100.0;
            ray4s[i].origin.x[k]    = u - 0.5;
            ray4s[i].origin.y[k]    = 0.0;
            ray4s[i].origin.z[k]    = 0.0;
            ray4s[i].direction.x[k] = 0.0;
            ray4s[i].direction.y[k] = 0.0;
            ray4s[i].direction.z[k] = -1.0;
        }
    }

    alignas(32) double r[4];
    for (auto _ : state) {
        int hits = 0;
        for (int i = 0; i < N / 4; i++) {
            hit_sphere4(ray4s[i], sphere, r);
            for (int k = 0; k < 4; k++)
                if (r[k] > 0.0) hits++;
        }
        benchmark::DoNotOptimize(hits);
    }
}
BENCHMARK(BM_HitSphereNAVX);
