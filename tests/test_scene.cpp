#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "scene.h"
#include "sphere.h"
#include "plane.h"

using Catch::Approx;

// Note : Scene a un constructeur de déplacement qui affiche sur stdout (TP11 pédagogique).
// C'est attendu — ça n'affecte pas la correction des tests.

static Material dummy_mat = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
    return Scatter{Vec3d(255, 255, 255), Ray(Vec3d(0,0,0), Vec3d(0,0,-1))};
};

// ── Scène vide ────────────────────────────────────────────────────────────────

TEST_CASE("Scene / scène vide retourne nullopt") {
    Scene scene;
    Ray r(Vec3d(0,0,0), Vec3d(0,0,-1));
    auto hit = scene.hit(r, 0.001, 1000.0);
    REQUIRE_FALSE(hit.has_value());
}

// ── Un seul objet ─────────────────────────────────────────────────────────────

TEST_CASE("Scene / une sphère touchée retourne un hit") {
    Scene scene;
    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-2), 0.5, dummy_mat));

    Ray r(Vec3d(0,0,0), Vec3d(0,0,-1));
    auto result = scene.hit(r, 0.001, 1000.0);

    REQUIRE(result.has_value());
    REQUIRE(result->first.t == Approx(1.5).epsilon(0.01));
}

TEST_CASE("Scene / une sphère manquée retourne nullopt") {
    Scene scene;
    scene.add(std::make_shared<Sphere>(Vec3d(10,0,-2), 0.5, dummy_mat));

    Ray r(Vec3d(0,0,0), Vec3d(0,0,-1));
    auto result = scene.hit(r, 0.001, 1000.0);

    REQUIRE_FALSE(result.has_value());
}

// ── Plusieurs objets ──────────────────────────────────────────────────────────

TEST_CASE("Scene / retourne l'intersection la plus proche") {
    Scene scene;
    // Deux sphères sur le même rayon
    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-5), 0.5, dummy_mat));  // loin (t≈4.5)
    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-2), 0.5, dummy_mat));  // proche (t≈1.5)

    Ray r(Vec3d(0,0,0), Vec3d(0,0,-1));
    auto result = scene.hit(r, 0.001, 1000.0);

    REQUIRE(result.has_value());
    REQUIRE(result->first.t == Approx(1.5).epsilon(0.01));
}

TEST_CASE("Scene / l'indice retourné correspond à l'objet touché") {
    Scene scene;
    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-5), 0.5, dummy_mat));  // indice 0 (loin)
    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-2), 0.5, dummy_mat));  // indice 1 (proche)

    Ray r(Vec3d(0,0,0), Vec3d(0,0,-1));
    auto result = scene.hit(r, 0.001, 1000.0);

    REQUIRE(result.has_value());
    REQUIRE(result->second == 1);  // la sphère proche est à l'indice 1
}

TEST_CASE("Scene / sphère devant masque sphère derrière") {
    Scene scene;
    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-2), 0.5, dummy_mat));  // bloque le rayon
    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-5), 0.5, dummy_mat));  // jamais atteinte

    Ray r(Vec3d(0,0,0), Vec3d(0,0,-1));
    auto result = scene.hit(r, 0.001, 1000.0);

    REQUIRE(result.has_value());
    REQUIRE(result->first.t < 3.0);  // c'est bien la sphère proche
}

TEST_CASE("Scene / sphère et plan — retourne le plus proche") {
    Scene scene;
    // Plan horizontal à y = -1
    scene.add(std::make_shared<Plane>(Vec3d(0,-1,0), Vec3d(0,1,0), dummy_mat));
    // Sphère au-dessus du plan
    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-2), 0.5, dummy_mat));

    // Rayon vers la sphère (pas vers le plan)
    Ray r_sphere(Vec3d(0,0,0), Vec3d(0,0,-1));
    auto hit_sphere = scene.hit(r_sphere, 0.001, 1000.0);
    REQUIRE(hit_sphere.has_value());
    REQUIRE(hit_sphere->first.t == Approx(1.5).epsilon(0.01));

    // Rayon vers le bas (vers le plan)
    Ray r_plane(Vec3d(0,5,0), Vec3d(0,-1,0));
    auto hit_plane = scene.hit(r_plane, 0.001, 1000.0);
    REQUIRE(hit_plane.has_value());
    REQUIRE(hit_plane->first.t == Approx(6.0).epsilon(0.01));
}

// ── add() et gestion des objets ───────────────────────────────────────────────

TEST_CASE("Scene / add() augmente le nombre d'objets") {
    Scene scene;
    REQUIRE(scene.objects.empty());

    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-1), 0.3, dummy_mat));
    REQUIRE(scene.objects.size() == 1);

    scene.add(std::make_shared<Sphere>(Vec3d(0,0,-3), 0.3, dummy_mat));
    REQUIRE(scene.objects.size() == 2);
}

TEST_CASE("Scene / objets accessibles via objects[]") {
    Scene scene;
    auto sphere = std::make_shared<Sphere>(Vec3d(1,2,3), 0.5, dummy_mat);
    scene.add(std::make_shared<Sphere>(Vec3d(1,2,3), 0.5, dummy_mat));

    REQUIRE(scene.objects.size() == 1);
    auto* s = dynamic_cast<Sphere*>(scene.objects[0].get());
    REQUIRE(s != nullptr);
    REQUIRE(s->radius == 0.5);
}
