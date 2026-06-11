#include <catch2/catch_test_macros.hpp>
#include "core/linear_allocator.h"
#include "core/object_pool.h"

using namespace rt;

// ═══════════════════════════════════════════════════════════════════════════════
// LinearAllocator
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("LinearAllocator / allocation de base retourne un pointeur non-nul", "[alloc]") {
    LinearAllocator alloc(1024);
    void* p = alloc.allocate(64);
    REQUIRE(p != nullptr);
}

TEST_CASE("LinearAllocator / alignement sur 16 respecté", "[alloc]") {
    LinearAllocator alloc(1024);
    void* p = alloc.allocate(64, 16);
    REQUIRE(reinterpret_cast<uintptr_t>(p) % 16 == 0);
}

TEST_CASE("LinearAllocator / alignement sur 32 respecté", "[alloc]") {
    LinearAllocator alloc(1024);
    void* p = alloc.allocate(64, 32);
    REQUIRE(reinterpret_cast<uintptr_t>(p) % 32 == 0);
}

TEST_CASE("LinearAllocator / deux allocations ne se chevauchent pas", "[alloc]") {
    LinearAllocator alloc(1024);
    void* p1 = alloc.allocate(64, 16);
    void* p2 = alloc.allocate(32, 16);
    REQUIRE(reinterpret_cast<std::byte*>(p2) >= reinterpret_cast<std::byte*>(p1) + 64);
}

TEST_CASE("LinearAllocator / used() augmente après allocation", "[alloc]") {
    LinearAllocator alloc(1024);
    REQUIRE(alloc.used() == 0);
    alloc.allocate(64);
    REQUIRE(alloc.used() >= 64);
}

TEST_CASE("LinearAllocator / total() retourne la capacité", "[alloc]") {
    LinearAllocator alloc(1024);
    REQUIRE(alloc.total() == 1024);
}

TEST_CASE("LinearAllocator / reset() remet used à zéro", "[alloc]") {
    LinearAllocator alloc(1024);
    alloc.allocate(64);
    alloc.reset();
    REQUIRE(alloc.used() == 0);
}

TEST_CASE("LinearAllocator / overflow lance std::bad_alloc", "[alloc]") {
    LinearAllocator alloc(32);
    REQUIRE_THROWS_AS(alloc.allocate(64), std::bad_alloc);
}

// ═══════════════════════════════════════════════════════════════════════════════
// ObjectPool
// ═══════════════════════════════════════════════════════════════════════════════

struct Counted {
    int value;
    int& destructor_calls;
    Counted(int v, int& counter) : value(v), destructor_calls(counter) {}
    ~Counted() { ++destructor_calls; }
};

TEST_CASE("ObjectPool / construct retourne un objet initialisé", "[alloc]") {
    int counter = 0;
    ObjectPool<Counted> pool(10);
    Counted* obj = pool.construct(42, counter);
    REQUIRE(obj != nullptr);
    REQUIRE(obj->value == 42);
}

TEST_CASE("ObjectPool / destroy_all appelle les destructeurs", "[alloc]") {
    int counter = 0;
    {
        ObjectPool<Counted> pool(10);
        pool.construct(1, counter);
        pool.construct(2, counter);
        pool.construct(3, counter);
        pool.destroy_all();
    }
    REQUIRE(counter == 3);
}

TEST_CASE("ObjectPool / destroy_all puis construct fonctionne (reset)", "[alloc]") {
    int counter = 0;
    ObjectPool<Counted> pool(10);
    pool.construct(1, counter);
    pool.destroy_all();
    REQUIRE(counter == 1);

    // Après reset, on peut reconstruire
    Counted* obj = pool.construct(99, counter);
    REQUIRE(obj != nullptr);
    REQUIRE(obj->value == 99);
}
