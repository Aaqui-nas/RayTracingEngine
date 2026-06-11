#include <benchmark/benchmark.h>
#include <memory_resource>
#include <vector>
#include <memory>
#include "core/linear_allocator.h"
#include "core/linear_resource.h"
#include "geometry/aabb.h"
#include "geometry/bvh.h"
#include "geometry/sphere.h"

using namespace rt;

static Material null_mat = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return std::nullopt;
};

// ── Allocation : LinearAllocator brut ────────────────────────────────────────

static void BM_Alloc_LinearAllocator(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        LinearAllocator alloc(N * sizeof(double));
        for (int i = 0; i < N; ++i) {
            double* p = static_cast<double*>(alloc.allocate(sizeof(double), alignof(double)));
            benchmark::DoNotOptimize(p);
        }
        alloc.reset();
    }
}
BENCHMARK(BM_Alloc_LinearAllocator)->Range(1000, 100000);

static void BM_Alloc_HeapNew(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        std::vector<double*> ptrs;
        ptrs.reserve(N);
        for (int i = 0; i < N; ++i)
            ptrs.push_back(new double);
        for (double* p : ptrs)
            delete p;
        benchmark::DoNotOptimize(ptrs);
    }
}
BENCHMARK(BM_Alloc_HeapNew)->Range(1000, 100000);

// ── pmr::vector : LinearResource vs monotonic vs standard ────────────────────

static void BM_Alloc_PmrLinearResource(benchmark::State& state) {
    const int N = state.range(0);
    LinearResource resource(N * sizeof(AABB) * 2);
    for (auto _ : state) {
        std::pmr::vector<AABB> v(&resource);
        v.reserve(N);
        AABB dummy{Vec3d(-1,-1,-1), Vec3d(1,1,1)};
        for (int i = 0; i < N; ++i)
            v.push_back(dummy);
        benchmark::DoNotOptimize(v);
        resource.reset();
    }
}
BENCHMARK(BM_Alloc_PmrLinearResource)->Range(1000, 100000);

static void BM_Alloc_PmrMonotonic(benchmark::State& state) {
    const int N = state.range(0);
    std::vector<std::byte> buf(N * sizeof(AABB) * 2);
    for (auto _ : state) {
        std::pmr::monotonic_buffer_resource resource(buf.data(), buf.size());
        std::pmr::vector<AABB> v(&resource);
        v.reserve(N);
        AABB dummy{Vec3d(-1,-1,-1), Vec3d(1,1,1)};
        for (int i = 0; i < N; ++i)
            v.push_back(dummy);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Alloc_PmrMonotonic)->Range(1000, 100000);

static void BM_Alloc_StdVector(benchmark::State& state) {
    const int N = state.range(0);
    for (auto _ : state) {
        std::vector<AABB> v;
        v.reserve(N);
        AABB dummy{Vec3d(-1,-1,-1), Vec3d(1,1,1)};
        for (int i = 0; i < N; ++i)
            v.push_back(dummy);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_Alloc_StdVector)->Range(1000, 100000);

// ── BVH build : pool vs heap ──────────────────────────────────────────────────

static std::shared_ptr<BVHNode> bvh_build_heap(
    std::vector<std::shared_ptr<Shape>>& shapes, int start, int end)
{
    int n = end - start;
    if (n == 1) {
        auto bb = shapes[start]->bounding_box().value();
        return std::make_shared<BVHNode>(shapes[start], shapes[start], bb);
    }
    if (n == 2) {
        auto bb = surrounding_box(shapes[start]->bounding_box().value(),
                                  shapes[start+1]->bounding_box().value());
        return std::make_shared<BVHNode>(shapes[start], shapes[start+1], bb);
    }
    int axis = rand() % 3;
    std::sort(shapes.begin()+start, shapes.begin()+end,
        [axis](const auto& a, const auto& b){
            return a->bounding_box().value().min_pt[axis]
                 < b->bounding_box().value().min_pt[axis];
        });
    int mid = start + n / 2;
    auto left  = bvh_build_heap(shapes, start, mid);
    auto right = bvh_build_heap(shapes, mid,   end);
    auto bb = surrounding_box(left->bounding_box().value(), right->bounding_box().value());
    return std::make_shared<BVHNode>(left, right, bb);
}

static void BM_BVH_Build_Heap(benchmark::State& state) {
    const int N = state.range(0);
    std::vector<std::shared_ptr<Shape>> shapes;
    shapes.reserve(N);
    for (int i = 0; i < N; ++i)
        shapes.push_back(std::make_shared<Sphere>(Vec3d(i*0.1, 0, -2), 0.4, null_mat));

    for (auto _ : state) {
        auto root = bvh_build_heap(shapes, 0, N);
        benchmark::DoNotOptimize(root);
    }
}
BENCHMARK(BM_BVH_Build_Heap)->Range(100, 10000);

static void BM_BVH_Build_Pool(benchmark::State& state) {
    const int N = state.range(0);
    std::vector<std::shared_ptr<Shape>> shapes;
    shapes.reserve(N);
    for (int i = 0; i < N; ++i)
        shapes.push_back(std::make_shared<Sphere>(Vec3d(i*0.1, 0, -2), 0.4, null_mat));

    for (auto _ : state) {
        BVHPool pool(N);
        BVHNode* root = pool.build(shapes, 0, N);
        benchmark::DoNotOptimize(root);
    }
}
BENCHMARK(BM_BVH_Build_Pool)->Range(100, 10000);
