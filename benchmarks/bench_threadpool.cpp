#include <benchmark/benchmark.h>
#include <atomic>
#include <numeric>
#include "core/thread_pool.h"
#include "core/tile_renderer.h"

using namespace rt;

// ── ThreadPool : overhead de submit ──────────────────────────────────────────

static void BM_ThreadPool_Submit_Overhead(benchmark::State& state) {
    ThreadPool pool(state.range(0));
    for (auto _ : state) {
        const int N = 1000;
        std::vector<std::future<int>> futures;
        futures.reserve(N);
        for (int i = 0; i < N; ++i)
            futures.push_back(pool.submit([i]{ return i * i; }));
        for (auto& f : futures) f.get();
    }
    state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(BM_ThreadPool_Submit_Overhead)->Arg(1)->Arg(2)->Arg(4)->Arg(8);

// ── ThreadPool : throughput calcul CPU ───────────────────────────────────────

static void BM_ThreadPool_CPU_Work(benchmark::State& state) {
    const int N_TASKS = state.range(0);
    ThreadPool pool(std::thread::hardware_concurrency());

    for (auto _ : state) {
        std::vector<std::future<long long>> futures;
        futures.reserve(N_TASKS);
        for (int i = 0; i < N_TASKS; ++i) {
            futures.push_back(pool.submit([i]{
                // Calcul léger pour simuler une tâche de rendu
                long long sum = 0;
                for (int j = 0; j < 1000; ++j)
                    sum += (i + j) * (i - j);
                return sum;
            }));
        }
        long long total = 0;
        for (auto& f : futures) total += f.get();
        benchmark::DoNotOptimize(total);
    }
    state.SetItemsProcessed(state.iterations() * N_TASKS);
}
BENCHMARK(BM_ThreadPool_CPU_Work)->Range(16, 1024);

// ── ThreadPool vs séquentiel ──────────────────────────────────────────────────

static void BM_Sequential_Work(benchmark::State& state) {
    const int N_TASKS = state.range(0);
    for (auto _ : state) {
        long long total = 0;
        for (int i = 0; i < N_TASKS; ++i) {
            for (int j = 0; j < 1000; ++j)
                total += (i + j) * (i - j);
        }
        benchmark::DoNotOptimize(total);
    }
    state.SetItemsProcessed(state.iterations() * N_TASKS);
}
BENCHMARK(BM_Sequential_Work)->Range(16, 1024);

// ── make_tiles ────────────────────────────────────────────────────────────────

static void BM_MakeTiles(benchmark::State& state) {
    const int tile_size = state.range(0);
    for (auto _ : state) {
        auto tiles = make_tiles(1920, 1080, tile_size, tile_size);
        benchmark::DoNotOptimize(tiles);
    }
}
BENCHMARK(BM_MakeTiles)->Arg(8)->Arg(16)->Arg(32)->Arg(64);
