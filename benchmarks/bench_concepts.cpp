#include <benchmark/benchmark.h>
#include <vector>
#include <memory>
#include <ranges>

#include "geometry/concepts.h"
#include "geometry/sphere.h"
#include "geometry/plane.h"
#include "scene/scene.h"
#include "materials/materials.h"

using namespace rt;

// ─── helpers ──────────────────────────────────────────────────────────────────

static Material dummy_mat() {
    return [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
        return std::nullopt;
    };
}

static std::vector<std::shared_ptr<Shape>> make_scene_ptrs(int n) {
    std::vector<std::shared_ptr<Shape>> v;
    v.reserve(n);
    for (int i = 0; i < n; ++i)
        v.push_back(std::make_shared<Sphere>(
            Vec3d(i * 0.5, 0, 0), 0.4, dummy_mat()));
    return v;
}

static std::vector<Sphere> make_sphere_values(int n) {
    std::vector<Sphere> v;
    v.reserve(n);
    for (int i = 0; i < n; ++i)
        v.emplace_back(Vec3d(i * 0.5, 0, 0), 0.4, dummy_mat());
    return v;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 3 — closest_hit : valeurs vs pointeurs
// ═══════════════════════════════════════════════════════════════════════════════

// Baseline : boucle manuelle sur shared_ptr<Shape> (dispatch virtuel)
static void BM_ClosestHit_VirtualDispatch(benchmark::State& state) {
    int n = state.range(0);
    auto objects = make_scene_ptrs(n);
    Ray ray(Vec3d(-1, 0.1, 0), Vec3d(1, 0, 0));

    for (auto _ : state) {
        std::optional<HitRecord> best;
        double closest = 1000.0;
        for (auto& obj : objects) {
            if (auto h = obj->hit(ray, 0.001, closest)) {
                closest = h->t;
                best    = h;
            }
        }
        benchmark::DoNotOptimize(best);
    }
}
BENCHMARK(BM_ClosestHit_VirtualDispatch)->Arg(8)->Arg(32)->Arg(128);

// Concept : closest_hit<Sphere> — pas de vtable, inlinable
static void BM_ClosestHit_Concept_Values(benchmark::State& state) {
    int n = state.range(0);
    auto spheres = make_sphere_values(n);
    Ray ray(Vec3d(-1, 0.1, 0), Vec3d(1, 0, 0));

    for (auto _ : state) {
        auto hit = closest_hit(spheres, ray, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_ClosestHit_Concept_Values)->Arg(8)->Arg(32)->Arg(128);

// shared_ptr wrappé dans closest_hit_ptrs
static void BM_ClosestHit_Concept_Ptrs(benchmark::State& state) {
    int n = state.range(0);
    auto objects = make_scene_ptrs(n);
    Ray ray(Vec3d(-1, 0.1, 0), Vec3d(1, 0, 0));

    for (auto _ : state) {
        auto hit = closest_hit_ptrs(objects, ray, 0.001, 1000.0);
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_ClosestHit_Concept_Ptrs)->Arg(8)->Arg(32)->Arg(128);

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 4 — std::views::filter : lazy vs eager copy
// ═══════════════════════════════════════════════════════════════════════════════

// Copie eager (vector filtré)
static void BM_EmissiveFilter_EagerCopy(benchmark::State& state) {
    int n = state.range(0);
    auto objects = make_scene_ptrs(n);
    // 1/4 des objets sont émissifs
    for (int i = 0; i < n; i += 4)
        objects[i]->emission = Vec3d(100, 100, 100);

    for (auto _ : state) {
        std::vector<std::shared_ptr<Shape>> result;
        for (auto& obj : objects)
            if (obj->emission.length() > 0.0)
                result.push_back(obj);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_EmissiveFilter_EagerCopy)->Arg(32)->Arg(128)->Arg(512);

// View lazy (std::views::filter — aucune allocation)
static void BM_EmissiveFilter_LazyView(benchmark::State& state) {
    int n = state.range(0);
    auto objects = make_scene_ptrs(n);
    for (int i = 0; i < n; i += 4)
        objects[i]->emission = Vec3d(100, 100, 100);

    for (auto _ : state) {
        auto view = emissive_shapes(objects);
        int count = 0;
        for ([[maybe_unused]] auto& e : view) ++count;
        benchmark::DoNotOptimize(count);
    }
}
BENCHMARK(BM_EmissiveFilter_LazyView)->Arg(32)->Arg(128)->Arg(512);

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 4c — std::ranges::sort vs std::sort manuel
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_SortByAxis_ManualSort(benchmark::State& state) {
    int n = state.range(0);

    for (auto _ : state) {
        state.PauseTiming();
        auto objects = make_scene_ptrs(n);
        state.ResumeTiming();

        std::sort(objects.begin(), objects.end(),
            [](const auto& a, const auto& b) {
                auto ba = a->bounding_box();
                auto bb = b->bounding_box();
                if (!ba || !bb) return false;
                return (ba->min_pt.x + ba->max_pt.x) < (bb->min_pt.x + bb->max_pt.x);
            });
        benchmark::DoNotOptimize(objects);
    }
}
BENCHMARK(BM_SortByAxis_ManualSort)->Arg(32)->Arg(128)->Arg(512);

static void BM_SortByAxis_RangesSort(benchmark::State& state) {
    int n = state.range(0);

    for (auto _ : state) {
        state.PauseTiming();
        auto objects = make_scene_ptrs(n);
        state.ResumeTiming();

        sort_by_axis(objects, 0);
        benchmark::DoNotOptimize(objects);
    }
}
BENCHMARK(BM_SortByAxis_RangesSort)->Arg(32)->Arg(128)->Arg(512);

// ═══════════════════════════════════════════════════════════════════════════════
// Concept check overhead : zéro à la compilation
// Mesure le coût d'appel d'une fonction contrainte vs non contrainte
// ═══════════════════════════════════════════════════════════════════════════════

static void BM_ConceptConstrained_Call(benchmark::State& state) {
    Sphere s(Vec3d(0, 0, 0), 1.0, dummy_mat());
    Ray ray(Vec3d(0, 0, 5), Vec3d(0, 0, -1));

    for (auto _ : state) {
        auto result = hittable_bbox(s);  // fonction contrainte par Bounded
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ConceptConstrained_Call);

static void BM_VirtualDispatch_Call(benchmark::State& state) {
    std::shared_ptr<Shape> s = std::make_shared<Sphere>(
        Vec3d(0, 0, 0), 1.0, dummy_mat());

    for (auto _ : state) {
        auto result = s->bounding_box();  // appel virtuel
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_VirtualDispatch_Call);
