#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <chrono>
#include <numeric>
#include "core/thread_pool.h"
#include "core/tile_renderer.h"

using namespace rt;

// ═══════════════════════════════════════════════════════════════════════════════
// ThreadPool
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("ThreadPool / submit retourne un future valide", "[threadpool]") {
    ThreadPool pool(2);
    auto f = pool.submit([]{ return 42; });
    REQUIRE(f.valid());
    REQUIRE(f.get() == 42);
}

TEST_CASE("ThreadPool / résultat correct pour un calcul simple", "[threadpool]") {
    ThreadPool pool(2);
    auto f = pool.submit([](int x, int y){ return x * y; }, 6, 7);
    REQUIRE(f.get() == 42);
}

TEST_CASE("ThreadPool / plusieurs futures résolus correctement", "[threadpool]") {
    ThreadPool pool(4);
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 20; ++i)
        futures.push_back(pool.submit([i]{ return i * i; }));

    for (int i = 0; i < 20; ++i)
        REQUIRE(futures[i].get() == i * i);
}

TEST_CASE("ThreadPool / tâches exécutées en parallèle", "[threadpool]") {
    const int N = 8;
    ThreadPool pool(N);
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;

    for (int i = 0; i < N; ++i) {
        futures.push_back(pool.submit([&counter]{
            ++counter;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }));
    }

    for (auto& f : futures) f.get();
    REQUIRE(counter.load() == N);
}

TEST_CASE("ThreadPool / wait_all bloque jusqu'à la fin de toutes les tâches", "[threadpool]") {
    ThreadPool pool(4);
    std::atomic<int> done{0};

    for (int i = 0; i < 16; ++i)
        pool.submit([&done]{ ++done; });

    pool.wait_all();
    REQUIRE(done.load() == 16);
}

TEST_CASE("ThreadPool / destructeur attend la fin des tâches", "[threadpool]") {
    std::atomic<int> done{0};
    {
        ThreadPool pool(2);
        for (int i = 0; i < 8; ++i)
            pool.submit([&done]{
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                ++done;
            });
    }   // destructeur appelé ici — doit attendre
    REQUIRE(done.load() == 8);
}

TEST_CASE("ThreadPool / submit après plusieurs wait_all fonctionne", "[threadpool]") {
    ThreadPool pool(2);
    auto f1 = pool.submit([]{ return 1; });
    pool.wait_all();
    auto f2 = pool.submit([]{ return 2; });
    REQUIRE(f1.get() == 1);
    REQUIRE(f2.get() == 2);
}

// ═══════════════════════════════════════════════════════════════════════════════
// make_tiles
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("make_tiles / image exactement divisible", "[threadpool]") {
    auto tiles = make_tiles(64, 64, 16, 16);
    REQUIRE(tiles.size() == 16u);   // 4×4 tuiles
}

TEST_CASE("make_tiles / image non-divisible — tuiles de bord plus petites", "[threadpool]") {
    auto tiles = make_tiles(50, 50, 16, 16);
    // 4 colonnes (0,16,32,48) × 4 lignes → 16 tuiles
    REQUIRE(tiles.size() == 16u);
    // La tuile en bas à droite doit finir exactement à (50,50)
    auto& last = tiles.back();
    REQUIRE(last.x1 == 50);
    REQUIRE(last.y1 == 50);
}

TEST_CASE("make_tiles / les tuiles couvrent toute l'image sans chevauchement", "[threadpool]") {
    const int W = 100, H = 80;
    auto tiles = make_tiles(W, H, 16, 16);

    std::vector<int> coverage(W * H, 0);
    for (const auto& t : tiles) {
        for (int y = t.y0; y < t.y1; ++y)
            for (int x = t.x0; x < t.x1; ++x)
                coverage[y * W + x]++;
    }

    bool all_covered = std::all_of(coverage.begin(), coverage.end(),
                                   [](int c){ return c == 1; });
    REQUIRE(all_covered);
}

TEST_CASE("make_tiles / une seule tuile si tile_size >= image", "[threadpool]") {
    auto tiles = make_tiles(32, 32, 64, 64);
    REQUIRE(tiles.size() == 1u);
    REQUIRE(tiles[0].x0 == 0);
    REQUIRE(tiles[0].y0 == 0);
    REQUIRE(tiles[0].x1 == 32);
    REQUIRE(tiles[0].y1 == 32);
}
