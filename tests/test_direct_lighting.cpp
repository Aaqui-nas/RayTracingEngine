#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <vector>
#include <random>
#include "lights/lights.h"
#include "scene/scene.h"
#include "geometry/sphere.h"
#include "materials/materials.h"

using namespace rt;
using Catch::Approx;

// ─── helpers ──────────────────────────────────────────────────────────────────

static Material dummy_mat() {
    return [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
        return Scatter{Vec3d(200,200,200), Ray(Vec3d(0,0,0), Vec3d(0,1,0))};
    };
}

// ═══════════════════════════════════════════════════════════════════════════════
// make_sphere_light — propriétés géométriques de base
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("make_sphere_light / sample présent") {
    Light l = make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(10, 10, 10));
    auto s = l.sample(Vec3d(0, 0, 0), 0.5, 0.5);
    REQUIRE(s.has_value());
}

TEST_CASE("make_sphere_light / direction normalisée") {
    Light l = make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(10, 10, 10));
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < 50; ++i) {
        auto s = l.sample(Vec3d(0, 0, 0), dist(gen), dist(gen));
        REQUIRE(s.has_value());
        REQUIRE(s->direction.length() == Approx(1.0).epsilon(1e-6));
    }
}

TEST_CASE("make_sphere_light / distance positive et bornée") {
    Vec3d center(0, 10, 0);
    double radius = 2.0;
    Light l = make_sphere_light(center, radius, Vec3d(5, 5, 5));
    Vec3d from(0, 0, 0);
    double d = (center - from).length();

    std::mt19937 gen(7);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < 50; ++i) {
        auto s = l.sample(from, dist(gen), dist(gen));
        REQUIRE(s.has_value());
        REQUIRE(s->distance > 0.0);
        REQUIRE(s->distance < d + radius + 0.1);
    }
}

TEST_CASE("make_sphere_light / pdf positif") {
    Light l = make_sphere_light(Vec3d(3, 0, 0), 0.5, Vec3d(8, 8, 8));
    std::mt19937 gen(13);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < 50; ++i) {
        auto s = l.sample(Vec3d(0, 0, 0), dist(gen), dist(gen));
        REQUIRE(s.has_value());
        REQUIRE(s->pdf > 0.0);
    }
}

TEST_CASE("make_sphere_light / radiance = émission") {
    Vec3d emission(5.0, 3.0, 1.0);
    Light l = make_sphere_light(Vec3d(0, 10, 0), 1.0, emission);
    auto s = l.sample(Vec3d(0, 0, 0), 0.3, 0.7);
    REQUIRE(s.has_value());
    REQUIRE(s->radiance.x == Approx(emission.x).epsilon(0.01));
    REQUIRE(s->radiance.y == Approx(emission.y).epsilon(0.01));
    REQUIRE(s->radiance.z == Approx(emission.z).epsilon(0.01));
}

TEST_CASE("make_sphere_light / direction dans le cône") {
    // Le centre est à (0, 8, 0), rayon = 0.5, observateur en (0, 0, 0)
    // cos_theta_max = sqrt(1 - (0.5/8)^2) ≈ 0.9980
    Vec3d center(0, 8, 0);
    double radius = 0.5;
    Vec3d from(0, 0, 0);
    Vec3d z = (center - from).normalized();

    Light l = make_sphere_light(center, radius, Vec3d(1, 1, 1));
    double cos_max = std::sqrt(1.0 - (radius / (center - from).length()) *
                                     (radius / (center - from).length()));

    std::mt19937 gen(99);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < 100; ++i) {
        auto s = l.sample(from, dist(gen), dist(gen));
        REQUIRE(s.has_value());
        double cos_theta = dot(s->direction, z);
        REQUIRE(cos_theta >= cos_max - 1e-5);
        REQUIRE(cos_theta <= 1.0 + 1e-5);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// make_sphere_light — cohérence sample / pdf
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("make_sphere_light / pdf(from, dir) cohérent avec sample.pdf") {
    Light l = make_sphere_light(Vec3d(0, 6, 0), 1.0, Vec3d(4, 4, 4));
    Vec3d from(0, 0, 0);
    std::mt19937 gen(55);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < 30; ++i) {
        auto s = l.sample(from, dist(gen), dist(gen));
        REQUIRE(s.has_value());
        double pdf_fn = l.pdf(from, s->direction);
        // Les deux valeurs de pdf doivent être cohérentes (même constante)
        REQUIRE(pdf_fn == Approx(s->pdf).epsilon(1e-4));
    }
}

TEST_CASE("make_sphere_light / intégrale PDF ≈ 1 (Monte Carlo)") {
    // ∫ pdf(ω) dω sur le cône = 1
    // Estimation : N directions uniformes dans le cône, moyenne × solid_angle ≈ 1
    Light l = make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(1, 1, 1));
    Vec3d from(0, 0, 0);
    std::mt19937 gen(77);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    const int N = 2000;
    double sum = 0.0;
    for (int i = 0; i < N; ++i) {
        auto s = l.sample(from, dist(gen), dist(gen));
        REQUIRE(s.has_value());
        // Estimateur: 1 / pdf × pdf = 1, donc sum / N → 1
        sum += 1.0;  // par définition : E[1/pdf × pdf] = 1
    }
    // Vérification indirecte : tous les samples ont la même pdf (uniforme)
    auto s0 = l.sample(from, 0.1, 0.1);
    auto s1 = l.sample(from, 0.9, 0.9);
    REQUIRE(s0->pdf == Approx(s1->pdf).epsilon(1e-6));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Scene::occluded
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Scene::occluded / scène vide — jamais bloqué") {
    Scene scene;
    REQUIRE_FALSE(scene.occluded(Vec3d(0, 0, 0), Vec3d(0, 5, 0)));
    REQUIRE_FALSE(scene.occluded(Vec3d(-1, 0, 0), Vec3d(3, 2, 1)));
}

TEST_CASE("Scene::occluded / sphère bloquant le trajet") {
    Scene scene;
    scene.add(std::make_shared<Sphere>(Vec3d(0, 2.5, 0), 1.0, dummy_mat()));
    // De (0, 0, 0) vers (0, 5, 0) : la sphère en (0, 2.5, 0) r=1 est sur le chemin
    REQUIRE(scene.occluded(Vec3d(0, 0.01, 0), Vec3d(0, 4.99, 0)));
}

TEST_CASE("Scene::occluded / sphère hors du trajet — non bloqué") {
    Scene scene;
    scene.add(std::make_shared<Sphere>(Vec3d(5, 2.5, 0), 1.0, dummy_mat()));
    // De (0, 0, 0) vers (0, 5, 0) : la sphère en x=5 n'est pas sur le chemin
    REQUIRE_FALSE(scene.occluded(Vec3d(0, 0.01, 0), Vec3d(0, 4.99, 0)));
}

TEST_CASE("Scene::occluded / sphère derrière la cible — non bloqué") {
    Scene scene;
    // Sphère à y=8, cible à y=5 → la sphère est au-delà de la cible
    scene.add(std::make_shared<Sphere>(Vec3d(0, 8, 0), 1.0, dummy_mat()));
    REQUIRE_FALSE(scene.occluded(Vec3d(0, 0.01, 0), Vec3d(0, 4.99, 0)));
}

TEST_CASE("Scene::occluded / réciproque — deux directions opposées") {
    Scene scene;
    // Une sphère en y=2 bloque (0→5) mais pas (5→0) si elle est entre eux
    scene.add(std::make_shared<Sphere>(Vec3d(0, 2.5, 0), 0.3, dummy_mat()));
    bool blocked_up   = scene.occluded(Vec3d(0, 0.5, 0), Vec3d(0, 4.5, 0));
    bool blocked_down = scene.occluded(Vec3d(0, 4.5, 0), Vec3d(0, 0.5, 0));
    REQUIRE(blocked_up   == true);
    REQUIRE(blocked_down == true);
}

// ═══════════════════════════════════════════════════════════════════════════════
// mis_weight
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("mis_weight / pdfs égaux → 0.5") {
    REQUIRE(mis_weight(1.0, 1.0) == Approx(0.5).epsilon(1e-9));
    REQUIRE(mis_weight(5.0, 5.0) == Approx(0.5).epsilon(1e-9));
}

TEST_CASE("mis_weight / symétrie — somme vaut 1") {
    std::vector<std::pair<double,double>> cases = {
        {1.0, 2.0}, {0.1, 0.9}, {3.0, 0.5}, {10.0, 0.01}
    };
    for (auto [a, b] : cases) {
        double wa = mis_weight(a, b);
        double wb = mis_weight(b, a);
        REQUIRE(wa + wb == Approx(1.0).epsilon(1e-9));
    }
}

TEST_CASE("mis_weight / un seul pdf nul") {
    REQUIRE(mis_weight(1.0, 0.0) == Approx(1.0).epsilon(1e-9));
    REQUIRE(mis_weight(0.0, 1.0) == Approx(0.0).epsilon(1e-9));
}

TEST_CASE("mis_weight / valeurs dans [0, 1]") {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist(0.0, 10.0);
    for (int i = 0; i < 100; ++i) {
        double a = dist(gen), b = dist(gen);
        double w = mis_weight(a, b);
        REQUIRE(w >= 0.0);
        REQUIRE(w <= 1.0);
    }
}

TEST_CASE("mis_weight / pdf_a > pdf_b → poids_a > 0.5") {
    REQUIRE(mis_weight(3.0, 1.0) > 0.5);
    REQUIRE(mis_weight(1.0, 3.0) < 0.5);
}

// ═══════════════════════════════════════════════════════════════════════════════
// LightList
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("LightList / vide → sample retourne nullopt") {
    LightList ll;
    REQUIRE(ll.empty());
    auto s = ll.sample(Vec3d(0, 0, 0), 0.5, 0.5, 0.5);
    REQUIRE_FALSE(s.has_value());
}

TEST_CASE("LightList / vide → pdf vaut 0") {
    LightList ll;
    REQUIRE(ll.pdf(Vec3d(0,0,0), Vec3d(0,1,0)) == Approx(0.0));
}

TEST_CASE("LightList / une lumière → retourne un sample valide") {
    LightList ll;
    ll.add(make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(10, 10, 10)));
    auto s = ll.sample(Vec3d(0, 0, 0), 0.5, 0.3, 0.7);
    REQUIRE(s.has_value());
    REQUIRE(s->direction.length() == Approx(1.0).epsilon(1e-6));
    REQUIRE(s->pdf > 0.0);
}

TEST_CASE("LightList / deux lumières identiques → pdf combinée = pdf_single") {
    // Mixture de N sources identiques : pdf_combinée = (1/N) × Σ pdf_i = pdf_single
    Light l1 = make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(10, 10, 10));
    Light l2 = make_sphere_light(Vec3d(0, 5, 0), 1.0, Vec3d(10, 10, 10));
    Vec3d from(0, 0, 0);

    auto s = l1.sample(from, 0.5, 0.5);
    REQUIRE(s.has_value());
    double pdf_single = s->pdf;

    LightList ll;
    ll.add(std::move(l1));
    ll.add(std::move(l2));

    // (1/2) × (pdf_single + pdf_single) = pdf_single
    double pdf_combined = ll.pdf(from, s->direction);
    REQUIRE(pdf_combined == Approx(pdf_single).epsilon(0.01));
}

TEST_CASE("LightList / sample.pdf cohérent avec pdf()") {
    LightList ll;
    ll.add(make_sphere_light(Vec3d(2, 4, 0), 0.5, Vec3d(8, 8, 8)));
    ll.add(make_sphere_light(Vec3d(-2, 4, 0), 0.5, Vec3d(8, 8, 8)));
    Vec3d from(0, 0, 0);

    std::mt19937 gen(31);
    std::uniform_real_distribution<double> d(0.0, 1.0);
    for (int i = 0; i < 30; ++i) {
        auto s = ll.sample(from, d(gen), d(gen), d(gen));
        REQUIRE(s.has_value());
        double p = ll.pdf(from, s->direction);
        // La pdf stockée dans le sample doit correspondre à la pdf combinée
        REQUIRE(s->pdf == Approx(p).epsilon(0.01));
    }
}

TEST_CASE("LightList / toutes les lumières échantillonnées") {
    // Deux lumières dans des directions opposées :
    // l'une à gauche (x=-6), l'autre à droite (x=+6)
    // Chaque lumière ne peut être visible que depuis la moitié des samples
    LightList ll;
    ll.add(make_sphere_light(Vec3d(-6, 0.01, 0), 0.5, Vec3d(1, 0, 0)));  // rouge
    ll.add(make_sphere_light(Vec3d( 6, 0.01, 0), 0.5, Vec3d(0, 0, 1)));  // bleu

    Vec3d from(0, 0, 0);
    std::mt19937 gen(17);
    std::uniform_real_distribution<double> d(0.0, 1.0);

    int count_left = 0, count_right = 0;
    const int N = 500;
    for (int i = 0; i < N; ++i) {
        auto s = ll.sample(from, d(gen), d(gen), d(gen));
        if (!s) continue;
        if (s->direction.x < 0) ++count_left;
        else                     ++count_right;
    }
    // Chaque lumière doit être choisie ≈ N/2 fois
    REQUIRE(count_left  > N / 5);
    REQUIRE(count_right > N / 5);
}
