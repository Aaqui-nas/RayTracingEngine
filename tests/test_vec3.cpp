#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "vec3.h"

using namespace rt;
using Catch::Approx;

// ── Construction ──────────────────────────────────────────────────────────────

TEST_CASE("Vec3 / construction par défaut initialise à zéro") {
    Vec3d v;
    REQUIRE(v.x == 0.0);
    REQUIRE(v.y == 0.0);
    REQUIRE(v.z == 0.0);
}

TEST_CASE("Vec3 / construction avec valeurs") {
    Vec3d v(1.0, 2.0, 3.0);
    REQUIRE(v.x == 1.0);
    REQUIRE(v.y == 2.0);
    REQUIRE(v.z == 3.0);
}

TEST_CASE("Vec3 / aliases de types") {
    Vec3d  d(1.0, 2.0, 3.0);
    Vec3f  f(1.0f, 2.0f, 3.0f);
    Vec3i  i(1, 2, 3);
    REQUIRE(d.x == 1.0);
    REQUIRE(f.x == 1.0f);
    REQUIRE(i.x == 1);
}

// ── Opérateurs arithmétiques ──────────────────────────────────────────────────

TEST_CASE("Vec3 / addition") {
    Vec3d a(1, 2, 3), b(4, 5, 6);
    Vec3d r = a + b;
    REQUIRE(r.x == 5.0);
    REQUIRE(r.y == 7.0);
    REQUIRE(r.z == 9.0);
}

TEST_CASE("Vec3 / soustraction") {
    Vec3d a(5, 7, 9), b(1, 2, 3);
    Vec3d r = a - b;
    REQUIRE(r.x == 4.0);
    REQUIRE(r.y == 5.0);
    REQUIRE(r.z == 6.0);
}

TEST_CASE("Vec3 / négation unaire") {
    Vec3d v(1, -2, 3);
    Vec3d r = -v;
    REQUIRE(r.x == -1.0);
    REQUIRE(r.y ==  2.0);
    REQUIRE(r.z == -3.0);
}

TEST_CASE("Vec3 / multiplication par scalaire") {
    Vec3d v(1, 2, 3);
    SECTION("v * s") {
        Vec3d r = v * 2.0;
        REQUIRE(r.x == 2.0);
        REQUIRE(r.y == 4.0);
        REQUIRE(r.z == 6.0);
    }
    SECTION("s * v (commutatif)") {
        Vec3d r = 2.0 * v;
        REQUIRE(r.x == 2.0);
        REQUIRE(r.y == 4.0);
        REQUIRE(r.z == 6.0);
    }
}

TEST_CASE("Vec3 / division par scalaire") {
    Vec3d v(2, 4, 6);
    Vec3d r = v / 2.0;
    REQUIRE(r.x == 1.0);
    REQUIRE(r.y == 2.0);
    REQUIRE(r.z == 3.0);
}

TEST_CASE("Vec3 / opérateurs d'assignation composés") {
    SECTION("+=") {
        Vec3d v(1, 2, 3);
        v += Vec3d(10, 20, 30);
        REQUIRE(v.x == 11.0);
        REQUIRE(v.y == 22.0);
        REQUIRE(v.z == 33.0);
    }
    SECTION("-=") {
        Vec3d v(10, 20, 30);
        v -= Vec3d(1, 2, 3);
        REQUIRE(v.x == 9.0);
        REQUIRE(v.y == 18.0);
        REQUIRE(v.z == 27.0);
    }
    SECTION("*=") {
        Vec3d v(1, 2, 3);
        v *= 3.0;
        REQUIRE(v.x == 3.0);
        REQUIRE(v.y == 6.0);
        REQUIRE(v.z == 9.0);
    }
    SECTION("/=") {
        Vec3d v(3, 6, 9);
        v /= 3.0;
        REQUIRE(v.x == 1.0);
        REQUIRE(v.y == 2.0);
        REQUIRE(v.z == 3.0);
    }
}

// ── Produit scalaire ──────────────────────────────────────────────────────────

TEST_CASE("Vec3 / produit scalaire (dot)") {
    SECTION("vecteurs perpendiculaires → 0") {
        Vec3d x(1, 0, 0), y(0, 1, 0);
        REQUIRE(dot(x, y) == Approx(0.0));
    }
    SECTION("vecteur avec lui-même → longueur²") {
        Vec3d v(3, 4, 0);
        REQUIRE(dot(v, v) == Approx(25.0));
    }
    SECTION("vecteurs opposés → négatif") {
        Vec3d a(1, 0, 0), b(-1, 0, 0);
        REQUIRE(dot(a, b) == Approx(-1.0));
    }
    SECTION("cas général") {
        Vec3d a(1, 2, 3), b(4, 5, 6);
        REQUIRE(dot(a, b) == Approx(32.0));  // 4+10+18
    }
}

// ── Produit vectoriel ─────────────────────────────────────────────────────────

TEST_CASE("Vec3 / produit vectoriel (cross) — règle de la main droite") {
    Vec3d x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);

    SECTION("x × y = z") {
        Vec3d r = x.cross(y);
        REQUIRE(r.x == Approx(0.0));
        REQUIRE(r.y == Approx(0.0));
        REQUIRE(r.z == Approx(1.0));
    }
    SECTION("y × z = x") {
        Vec3d r = y.cross(z);
        REQUIRE(r.x == Approx(1.0));
        REQUIRE(r.y == Approx(0.0));
        REQUIRE(r.z == Approx(0.0));
    }
    SECTION("z × x = y") {
        Vec3d r = z.cross(x);
        REQUIRE(r.x == Approx(0.0));
        REQUIRE(r.y == Approx(1.0));
        REQUIRE(r.z == Approx(0.0));
    }
    SECTION("anti-commutatif : a × b = -(b × a)") {
        Vec3d a(1, 2, 3), b(4, 5, 6);
        Vec3d ab = a.cross(b);
        Vec3d ba = b.cross(a);
        REQUIRE(ab.x == Approx(-ba.x));
        REQUIRE(ab.y == Approx(-ba.y));
        REQUIRE(ab.z == Approx(-ba.z));
    }
    SECTION("parallèles → vecteur nul") {
        Vec3d a(2, 0, 0), b(5, 0, 0);
        Vec3d r = a.cross(b);
        REQUIRE(dot(r, r) == Approx(0.0));
    }
}

// ── Longueur et normalisation ─────────────────────────────────────────────────

TEST_CASE("Vec3 / length") {
    SECTION("vecteur unitaire → 1") {
        REQUIRE(Vec3d(1, 0, 0).length() == Approx(1.0));
    }
    SECTION("triangle 3-4-5") {
        REQUIRE(Vec3d(3, 4, 0).length() == Approx(5.0));
    }
    SECTION("vecteur général") {
        Vec3d v(1, 2, 2);
        REQUIRE(v.length() == Approx(3.0));
    }
}

TEST_CASE("Vec3 / normalized produit un vecteur unitaire") {
    SECTION("vecteur quelconque") {
        Vec3d v(3, 4, 0);
        Vec3d n = v.normalized();
        REQUIRE(dot(n, n) == Approx(1.0));
    }
    SECTION("direction préservée") {
        Vec3d v(2, 0, 0);
        Vec3d n = v.normalized();
        REQUIRE(n.x == Approx(1.0));
        REQUIRE(n.y == Approx(0.0));
        REQUIRE(n.z == Approx(0.0));
    }
    SECTION("vecteur diagonal 3D") {
        Vec3d v(1, 1, 1);
        Vec3d n = v.normalized();
        double expected = 1.0 / std::sqrt(3.0);
        REQUIRE(n.x == Approx(expected));
        REQUIRE(n.y == Approx(expected));
        REQUIRE(n.z == Approx(expected));
    }
}
