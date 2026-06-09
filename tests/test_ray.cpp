#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "ray.h"

using Catch::Approx;

TEST_CASE("Ray / construction") {
    Vec3d origin(1, 2, 3);
    Vec3d dir(0, 0, -1);
    Ray r(origin, dir);

    REQUIRE(r.origin.x == 1.0);
    REQUIRE(r.origin.y == 2.0);
    REQUIRE(r.origin.z == 3.0);
    REQUIRE(r.direction.z == -1.0);
}

TEST_CASE("Ray / at(t)") {
    Ray r(Vec3d(0, 0, 0), Vec3d(1, 0, 0));

    SECTION("t=0 retourne l'origine") {
        Vec3d p = r.at(0.0);
        REQUIRE(p.x == Approx(0.0));
        REQUIRE(p.y == Approx(0.0));
        REQUIRE(p.z == Approx(0.0));
    }
    SECTION("t=1 retourne origine + direction") {
        Vec3d p = r.at(1.0);
        REQUIRE(p.x == Approx(1.0));
    }
    SECTION("t=2.5 scale correctement") {
        Vec3d p = r.at(2.5);
        REQUIRE(p.x == Approx(2.5));
    }
    SECTION("t négatif = derrière l'origine") {
        Vec3d p = r.at(-1.0);
        REQUIRE(p.x == Approx(-1.0));
    }
}

TEST_CASE("Ray / direction non normalisée est conservée") {
    Vec3d dir(2, 0, 0);
    Ray r(Vec3d(0, 0, 0), dir);
    Vec3d p = r.at(1.0);
    // at(1) = origin + 1 * direction = (2,0,0)
    REQUIRE(p.x == Approx(2.0));
}
