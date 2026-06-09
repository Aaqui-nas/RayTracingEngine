#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include "camera.h"

using namespace rt;
using Catch::Approx;

// ═══════════════════════════════════════════════════════════════════════════════
// Constructeur legacy — ne doit pas régresser
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Camera / constructeur par défaut produit un rayon valide") {
    Camera cam;
    Ray r = cam.get_ray(0.5, 0.5);
    REQUIRE(r.direction.length() > 0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// lookAt — orientation du rayon central
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Camera / rayon central pointe vers la cible (lookAt vers -z)") {
    Camera cam(Vec3d(0, 0, 0), Vec3d(0, 0, -1), Vec3d(0, 1, 0), 60.0, 16.0/9.0);
    Ray r = cam.get_ray(0.5, 0.5);
    Vec3d dir = r.direction.normalized();
    REQUIRE(dir.z < 0);
    REQUIRE(dir.x == Approx(0.0).margin(1e-6));
    REQUIRE(dir.y == Approx(0.0).margin(1e-6));
}

TEST_CASE("Camera / caméra latérale — rayon central pointe vers -x") {
    Camera cam(Vec3d(5, 0, 0), Vec3d(0, 0, 0), Vec3d(0, 1, 0), 60.0, 16.0/9.0);
    Ray r = cam.get_ray(0.5, 0.5);
    Vec3d dir = r.direction.normalized();
    REQUIRE(dir.x < 0);
    REQUIRE(dir.y == Approx(0.0).margin(1e-4));
    REQUIRE(dir.z == Approx(0.0).margin(1e-4));
}

TEST_CASE("Camera / caméra en hauteur — rayon central pointe vers le bas") {
    Camera cam(Vec3d(0, 5, 0), Vec3d(0, 0, 0), Vec3d(0, 0, -1), 60.0, 16.0/9.0);
    Ray r = cam.get_ray(0.5, 0.5);
    Vec3d dir = r.direction.normalized();
    REQUIRE(dir.y < 0);
    REQUIRE(dir.x == Approx(0.0).margin(1e-4));
    REQUIRE(dir.z == Approx(0.0).margin(1e-4));
}

TEST_CASE("Camera / caméra reculée — rayon central pointe vers z négatif") {
    Camera cam(Vec3d(0, 0, 5), Vec3d(0, 0, 0), Vec3d(0, 1, 0), 60.0, 16.0/9.0);
    Ray r = cam.get_ray(0.5, 0.5);
    Vec3d dir = r.direction.normalized();
    REQUIRE(dir.z < 0);
    REQUIRE(dir.x == Approx(0.0).margin(1e-4));
    REQUIRE(dir.y == Approx(0.0).margin(1e-4));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Position de la caméra
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Camera / aperture=0 — l'origine du rayon est la position de la caméra") {
    Vec3d pos(1, 2, 3);
    Camera cam(pos, Vec3d(0, 0, 0), Vec3d(0, 1, 0), 60.0, 16.0/9.0, 0.0, 5.0);
    Ray r = cam.get_ray(0.5, 0.5);
    REQUIRE(r.origin.x == Approx(pos.x).epsilon(1e-6));
    REQUIRE(r.origin.y == Approx(pos.y).epsilon(1e-6));
    REQUIRE(r.origin.z == Approx(pos.z).epsilon(1e-6));
}

// ═══════════════════════════════════════════════════════════════════════════════
// FOV
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Camera / FOV large produit des rayons plus divergents que FOV étroit") {
    Camera narrow(Vec3d(0,0,0), Vec3d(0,0,-1), Vec3d(0,1,0), 30.0,  1.0);
    Camera wide  (Vec3d(0,0,0), Vec3d(0,0,-1), Vec3d(0,1,0), 120.0, 1.0);

    Ray r_narrow = narrow.get_ray(1.0, 0.5);
    Ray r_wide   = wide.get_ray(1.0, 0.5);

    double angle_narrow = std::acos(std::abs(r_narrow.direction.normalized().z));
    double angle_wide   = std::acos(std::abs(r_wide.direction.normalized().z));
    REQUIRE(angle_wide > angle_narrow);
}

TEST_CASE("Camera / FOV=90° aspect=1 — bord horizontal à 45°") {
    // Avec fov=90° et aspect=1, le bord du cadre est à tan(45°)=1 → angle de 45°
    Camera cam(Vec3d(0,0,0), Vec3d(0,0,-1), Vec3d(0,1,0), 90.0, 1.0, 0.0, 1.0);
    Ray r = cam.get_ray(1.0, 0.5);
    Vec3d dir = r.direction.normalized();
    double angle = std::atan2(std::abs(dir.x), std::abs(dir.z));
    REQUIRE(angle == Approx(pi / 4.0).epsilon(0.01));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Profondeur de champ (DOF)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Camera / aperture=0 — get_ray est déterministe") {
    Camera cam(Vec3d(0,0,0), Vec3d(0,0,-1), Vec3d(0,1,0), 60.0, 16.0/9.0, 0.0, 2.0);
    Ray r1 = cam.get_ray(0.3, 0.7);
    Ray r2 = cam.get_ray(0.3, 0.7);
    REQUIRE(r1.origin.x    == Approx(r2.origin.x));
    REQUIRE(r1.origin.y    == Approx(r2.origin.y));
    REQUIRE(r1.direction.x == Approx(r2.direction.x));
    REQUIRE(r1.direction.y == Approx(r2.direction.y));
    REQUIRE(r1.direction.z == Approx(r2.direction.z));
}

TEST_CASE("Camera / aperture>0 — les origines des rayons varient") {
    Camera cam(Vec3d(0,0,0), Vec3d(0,0,-1), Vec3d(0,1,0), 60.0, 16.0/9.0, 0.5, 2.0);
    bool found_different = false;
    Ray first = cam.get_ray(0.5, 0.5);
    for (int i = 0; i < 50; i++) {
        Ray r = cam.get_ray(0.5, 0.5);
        if (std::abs(r.origin.x - first.origin.x) > 1e-9 ||
            std::abs(r.origin.y - first.origin.y) > 1e-9) {
            found_different = true;
            break;
        }
    }
    REQUIRE(found_different);
}

TEST_CASE("Camera / aperture>0 — origines dans le disque d'ouverture") {
    double aperture = 0.8;
    Camera cam(Vec3d(0,0,0), Vec3d(0,0,-1), Vec3d(0,1,0), 60.0, 16.0/9.0, aperture, 2.0);
    for (int i = 0; i < 100; i++) {
        Ray r = cam.get_ray(0.5, 0.5);
        double dist = std::sqrt(r.origin.x*r.origin.x + r.origin.y*r.origin.y);
        REQUIRE(dist <= aperture / 2.0 + 1e-9);
    }
}

TEST_CASE("Camera / aperture>0 — direction pointe toujours vers le plan de mise au point") {
    double focus_dist = 3.0;
    Camera cam(Vec3d(0,0,0), Vec3d(0,0,-1), Vec3d(0,1,0), 60.0, 1.0, 0.5, focus_dist);
    // Le point visé au centre (u=0.5, v=0.5) doit être à focus_dist sur l'axe -z
    // Pour cela, direction normalisée doit être telle que origin + t*dir atteint z=-focus_dist
    for (int i = 0; i < 20; i++) {
        Ray r = cam.get_ray(0.5, 0.5);
        // Le rayon doit passer par (0, 0, -focus_dist) au centre
        // t tel que origin.z + t*dir.z = -focus_dist
        Vec3d dir = r.direction;
        double t = (-focus_dist - r.origin.z) / dir.z;
        Vec3d focus_point = r.origin + t * dir;
        REQUIRE(focus_point.x == Approx(0.0).margin(1e-4));
        REQUIRE(focus_point.y == Approx(0.0).margin(1e-4));
    }
}
