#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <vector>

#include "geometry/procedural.h"
#include "geometry/mesh.h"

using namespace rt;

static Material dummy_mat() {
    return [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
        return std::nullopt;
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 1 — make_uv_grid
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("make_uv_grid / taille", "[procedural]") {
    auto grid = make_uv_grid(4, 3);
    REQUIRE(grid.size() == 12);
}

TEST_CASE("make_uv_grid / premier et dernier éléments", "[procedural]") {
    auto grid = make_uv_grid(4, 3);
    // k=0 → u=0, v=0
    CHECK_THAT(grid[0].x, Catch::Matchers::WithinAbs(0.0, 1e-9));
    CHECK_THAT(grid[0].y, Catch::Matchers::WithinAbs(0.0, 1e-9));
    // k=7 → i=2, j=1 → u=2/4=0.5, v=1/3≈0.333
    CHECK_THAT(grid[7].x, Catch::Matchers::WithinAbs(0.5,       1e-9));
    CHECK_THAT(grid[7].y, Catch::Matchers::WithinAbs(1.0 / 3.0, 1e-9));
}

TEST_CASE("make_uv_grid / coordonnées dans [0, 1]", "[procedural]") {
    auto grid = make_uv_grid(8, 8);
    for (const auto& p : grid) {
        CHECK(p.x >= 0.0);
        CHECK(p.x <  1.0 + 1e-9);
        CHECK(p.y >= 0.0);
        CHECK(p.y <  1.0 + 1e-9);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 2 — generate_torus
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("generate_torus / compte des triangles", "[procedural]") {
    int nu = 16, nv = 12;
    Mesh m = generate_torus(1.0, 0.3, nu, nv, dummy_mat());
    REQUIRE(m.triangles.size() == static_cast<size_t>(2 * nu * nv));
}

TEST_CASE("generate_torus / triangles non dégénérés", "[procedural]") {
    Mesh m = generate_torus(1.0, 0.3, 8, 6, dummy_mat());
    for (const auto& tri : m.triangles) {
        // Les sommets ne doivent pas être tous identiques
        bool degenerate = (tri.A == tri.B) || (tri.B == tri.C) || (tri.A == tri.C);
        REQUIRE_FALSE(degenerate);
    }
}

TEST_CASE("generate_torus / symétrie du maillage", "[procedural]") {
    // Le tore est centré en (0,0,0)
    Mesh m = generate_torus(1.0, 0.3, 16, 8, dummy_mat());
    Vec3d centroid(0, 0, 0);
    for (const auto& tri : m.triangles)
        centroid = centroid + (tri.A + tri.B + tri.C);
    int n = static_cast<int>(m.triangles.size()) * 3;
    centroid = centroid * (1.0 / n);
    CHECK_THAT(centroid.x, Catch::Matchers::WithinAbs(0.0, 0.01));
    CHECK_THAT(centroid.y, Catch::Matchers::WithinAbs(0.0, 0.01));
    CHECK_THAT(centroid.z, Catch::Matchers::WithinAbs(0.0, 0.01));
}

TEST_CASE("generate_torus / bounding box non nulle", "[procedural]") {
    Mesh m = generate_torus(1.5, 0.4, 12, 8, dummy_mat());
    auto bb = m.bounding_box();
    REQUIRE(bb.has_value());
    // La boîte doit contenir R+r dans les trois axes
    double extent = 1.5 + 0.4;
    CHECK(bb->max_pt.x  >  extent * 0.9);
    CHECK(bb->min_pt.x < -extent * 0.9);
}

TEST_CASE("generate_torus / ray hits torus", "[procedural]") {
    Mesh m = generate_torus(1.0, 0.3, 32, 16, dummy_mat());
    Ray ray(Vec3d(2.0, 0.0, 0.0), Vec3d(-1, 0, 0));
    auto hit = m.hit(ray, 0.001, 100.0);
    REQUIRE(hit.has_value());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 3 — generate_terrain
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("generate_terrain / compte des triangles", "[procedural]") {
    int N = 5, M = 4;
    std::vector<std::vector<double>> heights(N, std::vector<double>(M, 0.0));
    Mesh m = generate_terrain(heights, 1.0, 1.0, dummy_mat());
    REQUIRE(m.triangles.size() == static_cast<size_t>(2 * (N-1) * (M-1)));
}

TEST_CASE("generate_terrain / hauteurs respectées", "[procedural]") {
    // Un seul pic au centre
    std::vector<std::vector<double>> heights = {
        {0.0, 0.0, 0.0},
        {0.0, 5.0, 0.0},
        {0.0, 0.0, 0.0},
    };
    Mesh m = generate_terrain(heights, 1.0, 1.0, dummy_mat());
    auto bb = m.bounding_box();
    REQUIRE(bb.has_value());
    CHECK(bb->max_pt.y > 4.5);   // pic visible dans la bbox
    CHECK_THAT(bb->min_pt.y, Catch::Matchers::WithinAbs(0.0, 1e-4));
}

TEST_CASE("generate_terrain / flat terrain — tous les y = 0", "[procedural]") {
    std::vector<std::vector<double>> heights(4, std::vector<double>(4, 0.0));
    Mesh m = generate_terrain(heights, 1.0, 1.0, dummy_mat());
    for (const auto& tri : m.triangles) {
        CHECK_THAT(tri.A.y, Catch::Matchers::WithinAbs(0.0, 1e-9));
        CHECK_THAT(tri.B.y, Catch::Matchers::WithinAbs(0.0, 1e-9));
        CHECK_THAT(tri.C.y, Catch::Matchers::WithinAbs(0.0, 1e-9));
    }
}

TEST_CASE("generate_terrain / ray hits flat terrain", "[procedural]") {
    std::vector<std::vector<double>> heights(8, std::vector<double>(8, 0.0));
    Mesh m = generate_terrain(heights, 1.0, 1.0, dummy_mat());
    // Rayon tombant du dessus au milieu du terrain
    Ray ray(Vec3d(3.5, 5.0, 3.5), Vec3d(0, -1, 0));
    auto hit = m.hit(ray, 0.001, 100.0);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->point.y, Catch::Matchers::WithinAbs(0.0, 1e-4));
}

TEST_CASE("generate_terrain / scale cell_size", "[procedural]") {
    std::vector<std::vector<double>> heights(3, std::vector<double>(3, 0.0));
    Mesh m1 = generate_terrain(heights, 1.0, 1.0, dummy_mat());
    Mesh m2 = generate_terrain(heights, 2.0, 1.0, dummy_mat());
    auto bb1 = m1.bounding_box().value();
    auto bb2 = m2.bounding_box().value();
    // Doubler cell_size double l'étendue X et Z
    CHECK_THAT(bb2.max_pt.x, Catch::Matchers::WithinAbs(bb1.max_pt.x * 2.0, 2e-4));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 4 — generate_sphere_mesh
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("generate_sphere_mesh / triangle count", "[procedural]") {
    int nu = 8, nv = 8;
    Mesh m = generate_sphere_mesh(1.0, nu, nv, dummy_mat());
    // nu*nv quads → 2*nu*nv triangles (approximation pôles non comptés)
    REQUIRE(m.triangles.size() > 0);
}

TEST_CASE("generate_sphere_mesh / sommets sur la sphère", "[procedural]") {
    double radius = 2.0;
    Mesh m = generate_sphere_mesh(radius, 16, 16, dummy_mat());
    for (const auto& tri : m.triangles) {
        // Chaque sommet doit être à distance ≈ radius du centre
        CHECK_THAT(tri.A.length(), Catch::Matchers::WithinAbs(radius, 0.01));
        CHECK_THAT(tri.B.length(), Catch::Matchers::WithinAbs(radius, 0.01));
        CHECK_THAT(tri.C.length(), Catch::Matchers::WithinAbs(radius, 0.01));
    }
}

TEST_CASE("generate_sphere_mesh / ray hit", "[procedural]") {
    Mesh m = generate_sphere_mesh(1.0, 32, 16, dummy_mat());
    Ray ray(Vec3d(0, 0, 5), Vec3d(0, 0, -1));
    auto hit = m.hit(ray, 0.001, 100.0);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->point.z, Catch::Matchers::WithinAbs(1.0, 0.05));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Exercice 5 (bonus) — apply_noise
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("apply_noise / modifie les hauteurs", "[procedural][bonus]") {
    std::vector<std::vector<double>> heights(8, std::vector<double>(8, 0.0));
    apply_noise(heights, 0.1, 1.0);
    bool any_nonzero = false;
    for (const auto& row : heights)
        for (double h : row)
            if (std::abs(h) > 1e-9) { any_nonzero = true; break; }
    REQUIRE(any_nonzero);
}
