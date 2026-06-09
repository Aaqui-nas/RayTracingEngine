# Roadmap complète — Ray Tracer C++ (TP1 → TP60)

De zéro jusqu'au ray tracing interactif temps réel, en passant par tous les concepts C++ modernes.

Les durées sont des estimations réalistes pour un apprentissage en solo, débogage inclus.  
**Total estimé : ~620h** — soit ~15 mois à 10h/semaine, ou ~8 mois à 20h/semaine.

---

## Phase 1 — Fondations C++ (TP1–11) · ~34h

| TP | Titre | C++ introduit | Feature graphique | Durée |
|---|---|---|---|---|
| **TP1** | Variables, types, I/O | `int`, `double`, boucles `for`, `std::cout`, compilation `g++` | Premier dégradé de couleurs en PPM | ~2h |
| **TP2** | Fonctions, références, const | Fonctions, `const`, passage par référence, `return` | Couleur basée sur la direction d'un rayon fictif | ~2h |
| **TP3** | Struct, class, constructeurs | `struct`, `class`, constructeur, `this`, liste d'initialisation | `Vec3` et `Ray` | ~3h |
| **TP4** | Surcharge d'opérateurs | `operator+`, `-`, `*`, `/`, `dot`, `cross` | `Vec3` arithmétique complète | ~3h |
| **TP5** | Héritage et polymorphisme | `virtual`, héritage `public`, `override`, classes abstraites | Hiérarchie `Shape→Sphere/Plane`, premier rendu | ~4h |
| **TP6** | Templates | `template<typename T>`, instanciation implicite | `Vec3<T>` générique (`Vec3d`, `Vec3f`) | ~3h |
| **TP7** | Smart pointers, mémoire | `shared_ptr`, `unique_ptr`, `make_shared`, RAII | `Scene` avec liste d'objets, hit le plus proche | ~4h |
| **TP8** | STL — vector, algorithm | `std::vector`, range-for, `std::algorithm`, `std::map` | Gamma correction, buffer de pixels | ~3h |
| **TP9** | File I/O, parsing | `std::ifstream`, `std::istringstream`, `std::getline` | Scene loader depuis fichier `.txt` | ~4h |
| **TP10** | Multithreading | `std::thread`, `std::mutex`, `std::atomic` | Rendu parallèle par lignes | ~5h |
| **TP11** | Move semantics | Lvalue/rvalue, `std::move`, constructeur de déplacement | Optimisation des copies | ~4h |

---

## Phase 2 — C++ Moderne + Matériaux (TP12–12bis) · ~11h

| TP | Titre | C++ introduit | Feature graphique | Durée |
|---|---|---|---|---|
| **TP12** | Lambdas et C++17 | `auto`, lambdas, captures, `std::function`, `std::optional`, structured bindings | Matériaux diffuse/métal/verre, réfraction Schlick, anti-aliasing, émission | ~6h |
| **TP12bis** | Refactoring et architecture | CMake, `namespace`, split `.h`/`.cpp`, ODR rule, `inline` | Classe `Camera` paramétrique, `build_materials()` | ~5h |

---

## Phase 3 — Géométrie complexe (TP13–14) · ~18h

| TP | Titre | C++ introduit | Feature graphique | Durée |
|---|---|---|---|---|
| **TP13** | Triangle et OBJ | Möller-Trumbore, parsing OBJ, `emplace_back` | Primitive `Triangle`, `Mesh`, import OBJ | ~8h |
| **TP14** | AABB et BVH | `bounding_box()` virtuel, `operator[]`, `std::sort` lambda, récursivité | Boîtes englobantes, arbre BVH, O(n) → O(log n) | ~10h |

---

## Phase 4 — Rendu physique avancé (TP15–23) · ~58h

| TP | Titre | C++ introduit | Feature graphique | Durée |
|---|---|---|---|---|
| **TP15** | Camera libre | Constructeur paramétrique, `M_PI`, géométrie sphérique | `lookAt`, FOV configurable, depth of field | ~5h |
| **TP16** | Textures et UV | `std::vector<uint8_t>`, loader PPM, struct imbriquées | UV mapping sur triangles, textures image, Perlin noise | ~6h |
| **TP17** | Normal mapping | Matrice TBN (tangent/bitangent/normal), espace tangent | Normal maps sur meshes — relief sans géométrie supplémentaire | ~5h |
| **TP18** | IBL — Environment maps | Loader HDR (RGBE), `std::vector<float>`, importance sampling sphérique | Skybox HDR comme source de lumière, reflets d'environnement | ~8h |
| **TP19** | Éclairage direct | Lambdas complexes, MIS (multiple importance sampling) | Shadow rays, lumières d'aire, ombres douces, NEE | ~8h |
| **TP20** | Matériaux PBR | Closures multi-paramètres, namespace `physics` | BRDF Cook-Torrance, metalness/roughness/IOR physique | ~8h |
| **TP21** | C++20 Concepts | `concept`, `requires`, `std::ranges`, `auto` paramètre | Refactorer `Shape` avec des concepts — erreurs lisibles | ~5h |
| **TP22** | Géométrie procédurale | `std::generate`, lambdas géométriques | Sphere UV, tore, terrain depuis heightmap | ~5h |
| **TP23** | Projet intégrateur | Tout ce qui précède | Scène finale : caméra libre, IBL, PBR, normal maps, meshes | ~8h |

---

## Phase 5 — Performance et architecture (TP24–32) · ~92h

| TP | Titre | C++ introduit | Feature graphique | Durée |
|---|---|---|---|---|
| **TP24** | SIMD / optimisation CPU | Intrinsics SSE/AVX (`__m256d`), `alignas`, benchmarking `std::chrono` | Vec3 calculé 4/8 à la fois, 4–8× speedup | ~12h |
| **TP25** | Scene graph | `std::variant`, pattern visitor, CRTP | Transformations hiérarchiques translate/rotate/scale | ~8h |
| **TP26** | Templates avancés | SFINAE, `if constexpr`, type traits | Primitives génériques : box, cylindre, cône, CSG | ~8h |
| **TP27** | Allocateurs custom | Placement new, memory pool, `alignas`, `std::pmr` | Pool de BVH nodes — zéro allocation hot path | ~10h |
| **TP28** | Thread pool avancé | `std::future`, `std::promise`, `condition_variable`, work stealing | Tile-based rendering, load balancing adaptatif | ~8h |
| **TP29** | Post-processing + SSAO | `std::span`, pipeline fonctionnel, kernel convolution | Tone mapping ACES/Reinhard, bloom + occlusion ambiante screen-space | ~8h |
| **TP30** | Participating media | Intégration numérique, random walk | Brouillard volumétrique, fumée, raymarching | ~12h |
| **TP31** | GPU avec CUDA | `__device__`, `__global__`, `cudaMallocManaged` | Portage GPU, 100–1000× speedup | ~20h |
| **TP32** | LOD et Instancing | Proxy objects, `shared_ptr` partagé, index de LOD | Level of Detail automatique, 1000 instances sans 1000× la mémoire | ~6h |

---

## Phase 6 — Temps réel interactif (TP33–40) · ~98h

Le pivot majeur : on passe de "générer une image" à "afficher en continu".

| TP | Titre | C++ introduit | Feature graphique | Durée |
|---|---|---|---|---|
| **TP33** | Fenêtrage SDL2 | `find_package(SDL2)`, event loop, `SDL_Event`, callbacks | Fenêtre interactive, caméra au clavier/souris | ~8h |
| **TP34** | Progressive rendering | Double buffering, `std::chrono` frame timing, swap atomique | Accumulation : qualité s'améliore si immobile, reset si on bouge | ~5h |
| **TP35** | TAA — Temporal AA | Jitter de caméra, matrice de reprojection, accumulation temporelle avec rejet | Anti-aliasing temporel — même rendu qu'Unreal/Unity à 1 spp | ~12h |
| **TP36** | Hot reload | `std::filesystem::last_write_time`, thread de surveillance, mutex rechargement | Modifier `scene.txt`/matériaux pendant le rendu → rechargement automatique | ~5h |
| **TP37** | Render graph / Frame graph | Déclaration des passes et dépendances, tri topologique, gestion des transitions de ressources | Organiser shadow pass, G-buffer, SSAO, bloom dans un pipeline lisible et extensible | ~15h |
| **TP38** | Débruitage OIDN | FFI C/C++, `CMake ExternalProject` | 4 spp + filtre ≈ 256 spp — rendu "propre" interactif | ~8h |
| **TP39** | Compute shaders OpenGL | OpenGL/GLAD, GLSL compute, SSBO, texture2D | Ray tracing GPU entièrement en GLSL — 30–60 fps | ~15h |
| **TP40** | Vulkan + Hardware RTX | Vulkan, `VK_KHR_ray_tracing_pipeline`, BLAS/TLAS, `rayPayloadEXT` | Ray tracing hardware RTX/RDNA2, paradigme des moteurs AAA | ~30h |

> ⚠️ TP40 est le TP le plus difficile de la roadmap. Vulkan seul représente plusieurs semaines pour un développeur senior — prévoir de la patience.

---

## Phase 7 — GI temps réel état de l'art (TP41–45) · ~85h

Ces TPs implémentent les algorithmes d'Unreal Engine 5 Lumen, Cyberpunk 2077, Alan Wake 2.

| TP | Titre | C++ / Algo | Feature graphique | Durée |
|---|---|---|---|---|
| **TP41** | ReSTIR DI | Reservoir sampling, weighted reservoir sampling | Éclairage direct réutilisé spatialement et temporellement — ombres à 1 spp | ~20h |
| **TP42** | ReSTIR GI | Extension à l'indirect, path replay, temporal reuse | Illumination globale temps réel sans probes | ~20h |
| **TP43** | Hybrid rendering | Pipeline hybride G-buffer rasterisé + RT sélectif | Réflexions ray-tracées sur fond rasterisé — approche PS5/Series X | ~15h |
| **TP44** | Radiance Cascades | Structure cascade + interpolation bilinéaire | GI temps réel sans probe placement — style Lumen | ~20h |
| **TP45** | Atmosphère physique | Rayleigh + Mie scattering, LUT pré-calculées | Ciel dynamique physiquement correct | ~10h |

---

## Phase 8 — Algorithmes offline de référence (TP46–50) · ~74h

Algorithmes de film/VFX (Pixar, Weta, ILM). Moins "temps réel", mais indispensables pour comprendre POURQUOI ReSTIR marche.

| TP | Titre | C++ / Algo | Feature graphique | Durée |
|---|---|---|---|---|
| **TP46** | BDPT | Chemins bidirectionnels, MIS à N stratégies | Caustiques et éclairage indirect difficile (Cornell box) | ~20h |
| **TP47** | Photon Mapping | kd-tree des photons, gathering, density estimation | Caustiques nettes sous l'eau, foyer de loupe | ~15h |
| **TP48** | Rendu spectral | N longueurs d'onde au lieu de RGB, dispersion | Arc-en-ciel, prisme, caustiques chromatiques | ~12h |
| **TP49** | Participating media avancé | Équation de transfert radiatif, OpenVDB | Fumée/feu VDB (format Houdini), nuages volumétriques | ~15h |
| **TP50** | Neural denoising | ONNX Runtime, inference CPU/GPU | Denoiser réseau de neurones — qualité DLSS sur ton renderer | ~12h |

---

## Phase 9 — Éditeur de nœuds + moteur de jeu (TP51–60) · ~152h

| TP | Titre | C++ / Archi | Feature moteur | Durée |
|---|---|---|---|---|
| **TP51** | Scene editor — Dear ImGui | ImGui, docking, fenêtres flottantes | Éditeur graphique : sélectionner objets, modifier en live | ~8h |
| **TP52** | Node graph — data model | `std::variant` pour valeurs, DAG, tri topologique, visitor | Graphe de matériaux évaluable en C++ pur, sans GUI | ~12h |
| **TP53** | Visual node editor (imnodes) | `imnodes` via FetchContent, event handling, drag-and-drop | Éditeur visuel interactif — créer nœuds, tracer câbles, preview live | ~15h |
| **TP54** | Node material library | Factory pattern, interface abstraite, dispatch compile-time | Tous les nœuds Blender : BSDF, Mix, Texture, Math, Color Ramp… | ~12h |
| **TP55** | Asset pipeline — GLTF/Assimp | `CMake FetchContent`, parsers tiers, cache d'assets | Import GLTF 2.0 : maillages, matériaux PBR, squelettes, animations | ~10h |
| **TP56** | ECS — Entity Component System | Archetype storage, `std::tuple`, type erasure | Architecture données-orientées, base de tout moteur moderne | ~20h |
| **TP57** | Physics — Rigid body | Intégration Jolt Physics, collision shapes, raycasts | Boîtes qui tombent, contraintes, triggers | ~15h |
| **TP58** | Audio + Input | SDL_Mixer ou OpenAL, input polling, rebinding | Son 3D positionnel, clavier/souris/gamepad | ~8h |
| **TP59** | Scripting Lua | sol2/Lua 5.4, binding C++↔Lua, hot-reload scripts | Logique scriptée sans recompiler — comportements, cinématiques | ~12h |
| **TP60** | Projet final — Mini moteur | Tout assemblé | Jeu jouable : renderer RTX, éditeur nœuds, ECS, physique, scripting | ~40h |

---

## Vue macro complète

```
TP1–4    → C++ de base : types, classes, opérateurs             ~10h
TP5–8    → POO : héritage, templates, STL, mémoire              ~14h
TP9–11   → Production : I/O, threads, move semantics            ~13h
TP12     → C++ Moderne : lambdas, optional, structured bindings  ~6h
TP12bis  → Architecture : CMake, namespaces, séparation          ~5h
TP13–14  → Algorithmes : Möller-Trumbore, BVH O(log n)          ~18h
TP15–23  → Rendu physique : camera, IBL, normal maps, PBR       ~58h
TP24–32  → Performance : SIMD, scene graph, allocateurs, CUDA   ~92h
TP33–40  → Interactivité : fenêtre, TAA, render graph, Vulkan   ~98h
TP41–45  → GI état de l'art : ReSTIR, radiance cascades         ~85h
TP46–50  → Offline de référence : BDPT, photon, spectral        ~74h
TP51–60  → Éditeur + moteur : node editor, ECS, physique        ~152h
─────────────────────────────────────────────────────────────────
TOTAL                                                           ~625h
```

---

## Concepts C++ couverts au total

| Catégorie | Concepts |
|---|---|
| Types & expressions | variables, const, references, types primitifs |
| Fonctions | surcharge, templates de fonctions, lambdas, closures |
| POO | struct/class, héritage, virtual/override, CRTP |
| Mémoire | RAII, smart pointers, move semantics, allocateurs custom, placement new |
| Templates | généricité, SFINAE, `if constexpr`, type traits, `concept`/`requires` |
| STL | vector, map, optional, function, variant, span, ranges, filesystem |
| Concurrence | thread, mutex, atomic, future, condition_variable, thread pool |
| Build | g++, CMake, `find_package`, ExternalProject, FetchContent |
| C++17 | structured bindings, `if constexpr`, `std::optional`, `std::variant` |
| C++20 | concepts, ranges, `auto` param |
| Performance | SIMD intrinsics, benchmarking, memory pools, ECS archetype storage, LOD |
| Patterns | visitor, factory, DAG, topological sort, proxy, render graph |
| FFI | C/C++, CUDA, OpenGL, Vulkan, Lua, ONNX Runtime, Jolt, Assimp, imnodes |
| Maths | algèbre linéaire, probabilités, transport de lumière, intégration Monte Carlo |
