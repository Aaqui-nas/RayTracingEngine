# LearnCPP — Ray Tracer from Scratch

Cours progressif d'apprentissage du C++ moderne à travers la construction d'un ray tracer from scratch, jusqu'à un moteur de rendu temps réel avec éditeur de matériaux visuel.

## Objectif

Partir de zéro en C++ et arriver à :
- Un ray tracer CPU avec matériaux physiques (PBR, verre, émission, BVH)
- Un renderer temps réel interactif avec accumulation temporelle (TAA, ReSTIR)
- Un éditeur de matériaux visuel à nœuds (style Blender)
- Un mini moteur de jeu avec ECS, physique, scripting Lua

**~60 TPs progressifs** — chaque TP introduit un concept C++ et une feature graphique.  
Voir [`memory/roadmap_complet.md`](memory/roadmap_complet.md) pour la vue exhaustive.

---

## Prérequis

```bash
# Compilateur C++17
g++ --version   # >= 7.0

# Build system
cmake --version # >= 3.15
sudo apt install cmake   # si absent

# Bibliothèques (téléchargées automatiquement par CMake via FetchContent)
# - Catch2 v3    → tests unitaires
# - Google Benchmark v1.8 → microbenchmarks
```

---

## Structure

```
LearnCPP/
├── CMakeLists.txt       ← build principal
├── subjects/            ← sujets de TP (TP1.md → TP14.md + suite)
├── tests/               ← tests unitaires (Catch2)
├── benchmarks/          ← microbenchmarks (Google Benchmark)
├── memory/              ← roadmap et notes de projet
├── main.cpp             ← code actif (évolue à chaque TP)
├── vec3.h               ← vecteur 3D générique
├── ray.h                ← rayon (origine + direction)
├── shape.h              ← classe de base + HitRecord + Material
├── sphere.h             ← sphère
├── plane.h              ← plan infini
├── scene.h              ← scène (liste d'objets + hit le plus proche)
├── scene_loader.h       ← chargement depuis scene.txt
└── scene.txt            ← scène courante
```

**Principe** : un seul répertoire de code qui évolue en place. Chaque TP modifie les fichiers existants — pas de dossiers `TP1/`, `TP2/` séparés.

---

## Build

```bash
cmake -S . -B build
cmake --build build --parallel
```

---

## Lancer le ray tracer

```bash
# Rendu standard (400×225, 64 samples)
./build/raytracer > image.ppm

# Avec paramètres : multiplicateur de résolution et nombre de samples
./build/raytracer 2.0 128 > image.ppm   # 800×450, 128 samples
./build/raytracer 0.5 16  > image.ppm   # 200×112, 16 samples (rapide)

# Visualiser
eog image.ppm   # ou feh, gimp, etc.
```

---

## Tests unitaires

```bash
cmake --build build --parallel
cd build && ctest --output-on-failure

# Ou directement
./build/tests/run_tests
./build/tests/run_tests "[Vec3]"          # filtrer par tag
./build/tests/run_tests "Sphere*"         # filtrer par nom
```

Couverture actuelle (TP1–12) :
- `test_vec3` — construction, arithmétique, dot, cross, length, normalized
- `test_ray` — construction, at(t)
- `test_shapes` — Sphere et Plane : hit/miss, front/back face, t min/max
- `test_scene` — scène vide, un objet, plusieurs objets (plus proche, masquage)
- `test_materials` — lambertian, métal, verre, black

---

## Benchmarks

```bash
./build/benchmarks/run_benchmarks

# Filtrer
./build/benchmarks/run_benchmarks --benchmark_filter=BM_Vec3
./build/benchmarks/run_benchmarks --benchmark_filter=BM_Scene_Hit

# Résultats de référence (Intel Core i7, O3) :
# BM_Vec3_Dot          ~0.14 ns   (3 FMAs, pratiquement gratuit)
# BM_Sphere_Hit        ~6.6 ns    (hit) / ~1.6 ns (miss)
# BM_Scene_Hit/10      ~58 ns
# BM_Scene_Hit/10000   ~42 µs     → O(n) confirmé → voir BVH (TP14)
```

---

## Progression des TPs

| Phase | TPs | Contenu |
|-------|-----|---------|
| 1 | TP1–11 | Fondations C++ : types, classes, templates, STL, threads |
| 2 | TP12–12bis | C++ moderne : lambdas, optional, CMake, namespaces |
| 3 | TP13–14 | Géométrie : triangle, OBJ, BVH O(log n) |
| 4 | TP15–23 | Rendu physique : caméra libre, IBL, normal maps, PBR |
| 5 | TP24–32 | Performance : SIMD, allocateurs, CUDA, LOD |
| 6 | TP33–40 | Temps réel : SDL2, TAA, render graph, Vulkan RTX |
| 7–8 | TP41–50 | GI avancé : ReSTIR, BDPT, photon mapping, spectral |
| 9 | TP51–60 | Moteur : éditeur de nœuds, ECS, physique, scripting |

---

## État actuel

Code à l'état **post-TP12** :
- Anti-aliasing (multi-sampling)
- Matériaux : diffuse, métal (fuzz), verre (Snell + Schlick), checker, verre teinté
- Source de lumière émissive
- Rendu multithreadé
- Chargement de scène depuis fichier texte
