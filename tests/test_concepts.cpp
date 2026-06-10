#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <vector>
#include <memory>
#include <ranges>

#include "geometry/concepts.h"
#include "geometry/sphere.h"
#include "geometry/plane.h"
#include "geometry/triangle.h"
#include "scene/scene.h"
#include "materials/materials.h"

using namespace rt;

// ─── helpers ──────────────────────────────────────────────────────────────────

static Material dummy_mat() {
    return [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
        return std::nullopt;
    };
}

static Sphere make_sphere(Vec3d center, double r = 0.5) {
    return Sphere(center, r, dummy_mat());
}

static std::shared_ptr<Sphere> make_sphere_ptr(Vec3d center, double r = 0.5) {
    return std::make_shared<Sphere>(center, r, dummy_mat());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 1 — Concept Hittable
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Hittable concept — satisfied by ray tracer shapes", "[concepts]") {
    SECTION("Sphere satisfies Hittable") {
        STATIC_REQUIRE(Hittable<Sphere>);
    }
    SECTION("Plane satisfies Hittable") {
        STATIC_REQUIRE(Hittable<Plane>);
    }
    SECTION("Triangle satisfies Hittable") {
        STATIC_REQUIRE(Hittable<Triangle>);
    }
    SECTION("int does not satisfy Hittable") {
        STATIC_REQUIRE_FALSE(Hittable<int>);
    }
    SECTION("double does not satisfy Hittable") {
        STATIC_REQUIRE_FALSE(Hittable<double>);
    }
    SECTION("Vec3d does not satisfy Hittable") {
        STATIC_REQUIRE_FALSE(Hittable<Vec3d>);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 2 — Concept Bounded
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Bounded concept — requires Hittable + bounding_box", "[concepts]") {
    SECTION("Sphere satisfies Bounded") {
        STATIC_REQUIRE(Bounded<Sphere>);
    }
    SECTION("Triangle satisfies Bounded") {
        STATIC_REQUIRE(Bounded<Triangle>);
    }
    // Note: Plane hérite bounding_box() de Shape → satisfait syntaxiquement Bounded
    // même si elle retourne nullopt à l'exécution.
    SECTION("Plane satisfies Bounded (inherited method, nullopt at runtime)") {
        STATIC_REQUIRE(Bounded<Plane>);
    }
    SECTION("int does not satisfy Bounded") {
        STATIC_REQUIRE_FALSE(Bounded<int>);
    }
    SECTION("Bounded implies Hittable (subsomption)") {
        STATIC_REQUIRE(Hittable<Sphere>);
        STATIC_REQUIRE(Bounded<Sphere>);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 3 — closest_hit sur un range de valeurs
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("closest_hit — works on vector<Sphere> (no shared_ptr)", "[concepts]") {
    Ray ray(Vec3d(0, 0, 5), Vec3d(0, 0, -1));

    std::vector<Sphere> spheres = {
        make_sphere(Vec3d(0, 0, 0)),   // z=0 → t≈4.5
        make_sphere(Vec3d(0, 0, 2)),   // z=2 → t≈2.5  ← plus proche
        make_sphere(Vec3d(0, 0, -2)),  // z=-2 → t≈6.5
    };

    auto hit = closest_hit(spheres, ray, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->point.z, Catch::Matchers::WithinAbs(2.5, 0.1));
}

TEST_CASE("closest_hit — works on vector<shared_ptr<Shape>>", "[concepts]") {
    Ray ray(Vec3d(0, 0, 5), Vec3d(0, 0, -1));

    std::vector<std::shared_ptr<Shape>> objects = {
        make_sphere_ptr(Vec3d(0, 0, 0)),
        make_sphere_ptr(Vec3d(0, 0, 2)),
    };

    // closest_hit_ptrs doit être défini dans concepts.h (exercice 3)
    auto hit = closest_hit_ptrs(objects, ray, 0.001, 1000.0);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->point.z, Catch::Matchers::WithinAbs(2.5, 0.1));
}

TEST_CASE("closest_hit — returns nullopt when no intersection", "[concepts]") {
    Ray ray(Vec3d(10, 10, 5), Vec3d(0, 0, -1));
    std::vector<Sphere> spheres = { make_sphere(Vec3d(0, 0, 0)) };

    auto hit = closest_hit(spheres, ray, 0.001, 1000.0);
    REQUIRE_FALSE(hit.has_value());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 4 — std::ranges views
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("emissive_shapes — filters non-emissive objects", "[concepts]") {
    std::vector<std::shared_ptr<Shape>> objects;

    auto s1 = make_sphere_ptr(Vec3d(0,0,0));
    auto s2 = make_sphere_ptr(Vec3d(1,0,0));
    auto s3 = make_sphere_ptr(Vec3d(2,0,0));
    s2->emission = Vec3d(100, 100, 100);

    objects.push_back(s1);
    objects.push_back(s2);
    objects.push_back(s3);

    auto emissive = emissive_shapes(objects);
    int count = 0;
    for (auto& e : emissive) {
        count++;
        CHECK(e->emission.length() > 0.0);
    }
    CHECK(count == 1);
}

TEST_CASE("bounded_shapes — filters objects whose bounding_box returns nullopt", "[concepts]") {
    std::vector<std::shared_ptr<Shape>> objects;

    objects.push_back(make_sphere_ptr(Vec3d(0, 0, 0)));
    objects.push_back(std::make_shared<Plane>(
        Vec3d(0,0,0), Vec3d(0,1,0), dummy_mat()));  // retourne nullopt
    objects.push_back(make_sphere_ptr(Vec3d(1, 0, 0)));

    auto bounded = bounded_shapes(objects);
    int count = 0;
    for ([[maybe_unused]] auto& b : bounded) count++;
    CHECK(count == 2);
}

TEST_CASE("sort_by_axis — sorts shapes along given axis", "[concepts]") {
    std::vector<std::shared_ptr<Shape>> objects = {
        make_sphere_ptr(Vec3d(3, 0, 0)),
        make_sphere_ptr(Vec3d(1, 0, 0)),
        make_sphere_ptr(Vec3d(2, 0, 0)),
    };

    sort_by_axis(objects, 0);

    auto cx = [](const std::shared_ptr<Shape>& s) {
        auto bb = s->bounding_box().value();
        return (bb.min_pt.x + bb.max_pt.x) * 0.5;
    };
    CHECK(cx(objects[0]) < cx(objects[1]));
    CHECK(cx(objects[1]) < cx(objects[2]));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 5 — static_assert avec message
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("register_shape — accepts valid shapes, rejects bad types", "[concepts]") {
    Sphere s = make_sphere(Vec3d(0,0,0));
    register_shape(s);
    // register_shape(42);  // ← décommente pour voir le message d'erreur
    SUCCEED("register_shape accepts Sphere");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 6 — Concept Renderable
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Renderable concept — requires material field", "[concepts]") {
    SECTION("Sphere is Renderable") {
        STATIC_REQUIRE(Renderable<Sphere>);
    }
    SECTION("Plane is Renderable") {
        STATIC_REQUIRE(Renderable<Plane>);
    }
    SECTION("Triangle is Renderable") {
        STATIC_REQUIRE(Renderable<Triangle>);
    }
    SECTION("int is not Renderable") {
        STATIC_REQUIRE_FALSE(Renderable<int>);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 7 (bonus) — build_bvh avec std::ranges
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Scene::build_bvh — same result with ranges refactor", "[concepts][bonus]") {
    Scene scene;
    scene.add(make_sphere_ptr(Vec3d(0, 0, 0)));
    scene.add(make_sphere_ptr(Vec3d(2, 0, 0)));
    scene.add(std::make_shared<Plane>(
        Vec3d(0,-1,0), Vec3d(0,1,0), dummy_mat()));

    scene.build_bvh();

    Ray ray(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    auto hit = scene.hit(ray, 0.001, 1000.0);
    REQUIRE(hit.has_value());
}