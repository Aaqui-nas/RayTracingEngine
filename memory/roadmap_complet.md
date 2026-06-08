# Roadmap complète — Ray Tracer C++ (TP1 → TP33)

De zéro jusqu'au ray tracing interactif temps réel, en passant par tous les concepts C++ modernes.

---

## Phase 1 — Fondations C++ (TP1–11)

| TP | Titre | C++ introduit | Feature graphique |
|---|---|---|---|
| **TP1** | Variables, types, I/O | `int`, `double`, boucles `for`, `std::cout`, compilation `g++` | Premier dégradé de couleurs en PPM |
| **TP2** | Fonctions, références, const | Fonctions, `const`, passage par référence, `return` | Couleur basée sur la direction d'un rayon fictif |
| **TP3** | Struct, class, constructeurs | `struct`, `class`, constructeur, `this`, liste d'initialisation | `Vec3` (vecteur 3D) et `Ray` (origine + direction) |
| **TP4** | Surcharge d'opérateurs | `operator+`, `-`, `*`, `/`, produit scalaire `dot`, produit vectoriel `cross` | `Vec3` pleinement opérationnel, `Ray::at(t)` simplifié |
| **TP5** | Héritage et polymorphisme | `virtual`, héritage `public`, `override`, classes abstraites | Hiérarchie `Shape→Sphere/Plane`, premier rendu sphère + plan |
| **TP6** | Templates | `template<typename T>`, instanciation implicite, spécialisation | `Vec3<T>` générique (`Vec3d`, `Vec3f`, `Vec3i`) |
| **TP7** | Smart pointers, mémoire | `shared_ptr`, `unique_ptr`, `make_shared`, RAII | `Scene` contenant une liste d'objets, hit le plus proche |
| **TP8** | STL — vector, algorithm | `std::vector`, range-for, `std::algorithm`, `std::map` | Gamma correction, gestion du buffer de pixels |
| **TP9** | File I/O, parsing | `std::ifstream`, `std::istringstream`, `std::getline` | Scene loader : charger une scène depuis un fichier `.txt` |
| **TP10** | Multithreading | `std::thread`, `std::mutex`, `std::atomic`, `hardware_concurrency` | Rendu parallèle : chaque thread traite un bloc de lignes |
| **TP11** | Move semantics | Lvalue/rvalue, `std::move`, constructeur de déplacement, `noexcept` | Optimisation des copies de `Scene` et de `Vec3` |

---

## Phase 2 — C++ Moderne + Matériaux (TP12–12bis)

| TP | Titre | C++ introduit | Feature graphique |
|---|---|---|---|
| **TP12** | Lambdas et C++17 | `auto`, lambdas, captures, `std::function`, `std::optional`, structured bindings | Matériaux diffuse/métal/verre, réfraction Schlick, anti-aliasing, source de lumière émissive |
| **TP12bis** | Refactoring et architecture | CMake, `namespace`, split `.h`/`.cpp`, ODR rule, `inline` | Classe `Camera` paramétrique, `build_materials()`, structure maintenable |

---

## Phase 3 — Géométrie complexe (TP13–14)

| TP | Titre | C++ introduit | Feature graphique |
|---|---|---|---|
| **TP13** | Triangle et OBJ | Algorithme implémenté (Möller-Trumbore), parsing OBJ (`stoi`, `substr`), `emplace_back` | Primitive `Triangle`, `Mesh` = collection de triangles, import de n'importe quel modèle 3D |
| **TP14** | AABB et BVH | `virtual` override de `bounding_box()`, `operator[]` sur template, `std::sort` avec lambda, récursivité | Boîtes englobantes, arbre BVH, passage de O(n) à O(log n) par rayon |

---

## Phase 4 — Rendu physique avancé (TP15–20)

| TP | Titre | C++ introduit | Feature graphique |
|---|---|---|---|
| **TP15** | Camera libre | Constructeur paramétrique avancé, `M_PI`, géométrie sphérique | `lookAt(from, at, up)`, FOV configurable, depth of field (bokeh) |
| **TP16** | Textures et UV | Struct imbriquées, `std::vector<uint8_t>`, loader PPM minimal | UV mapping sur triangles, textures image, textures procédurales (noise) |
| **TP17** | Éclairage direct | Lambdas avec captures complexes, MIS (multiple importance sampling) | Shadow rays, lumières d'aire, ombres douces correctes, NEE |
| **TP18** | Matériaux PBR | Closures capturant plusieurs paramètres, namespace `physics` | BRDF Cook-Torrance, metalness/roughness/IOR physique, Fresnel |
| **TP19** | C++20 Concepts | `concept`, `requires`, `std::ranges`, `auto` paramètre | Refactorer `Shape` avec des concepts — erreurs de compile lisibles |
| **TP20** | Projet intégrateur | Tout ce qui précède | Scène finale : caméra libre, textures, PBR, meshes, BVH, lumières |

---

## Phase 5 — Performance et architecture (TP21–28)

| TP | Titre | C++ introduit | Feature graphique |
|---|---|---|---|
| **TP21** | SIMD / optimisation CPU | Intrinsics SSE/AVX (`__m256d`), `alignas`, benchmarking avec `std::chrono` | `Vec3` calculé 4/8 à la fois, 4–8× speedup sur les intersections |
| **TP22** | Scene graph | `std::variant`, pattern visitor, CRTP | Transformations hiérarchiques : translate/rotate/scale sans copier les vertices |
| **TP23** | Templates avancés | SFINAE, `if constexpr`, type traits (`std::is_base_of`) | Primitives génériques : box, cylindre, cône, toute CSG |
| **TP24** | Allocateurs custom | Placement new, memory pool, `alignas`, `std::pmr` | Pool de BVH nodes — zéro allocation sur le hot path de rendu |
| **TP25** | Thread pool avancé | `std::future`, `std::promise`, `condition_variable`, work stealing | Tile-based rendering, load balancing adaptatif selon la complexité |
| **TP26** | Post-processing | `std::span`, pipeline fonctionnel, `std::transform` | Tone mapping ACES/Reinhard, bloom, aberration chromatique |
| **TP27** | Participating media | Intégration numérique (quadrature), random walk, VDB | Brouillard volumétrique, fumée, nuages — raymarching dans le volume |
| **TP28** | GPU avec CUDA | Interop C++/CUDA, `__device__`, `__global__`, `cudaMallocManaged` | Portage du ray tracer sur GPU, 100–1000× speedup |

---

## Phase 6 — Temps réel interactif (TP29–33)

Le pivot majeur : on passe de "générer une image" à "afficher en continu".  
Stratégie : 1 sample/pixel en temps réel + accumulation quand la caméra est immobile.

| TP | Titre | C++ introduit | Feature graphique |
|---|---|---|---|
| **TP29** | Fenêtrage SDL2 | Bibliothèque tierce (`find_package(SDL2)`), event loop, callbacks, `SDL_Event` | Fenêtre interactive, caméra déplaçable au clavier/souris, rendu live |
| **TP30** | Progressive rendering | Double buffering, `std::chrono` frame timing, swap atomique | Accumulation multi-frames : qualité s'améliore si immobile, reset si on bouge |
| **TP31** | Débruitage | FFI C/C++, `CMake ExternalProject`, Intel OIDN intégration | 4 spp + filtre ≈ qualité visuelle de 256 spp — rendu "propre" interactif |
| **TP32** | Compute shaders OpenGL | OpenGL/GLAD, GLSL compute shader, SSBO, texture2D, hot-reload shader | Ray tracing entièrement sur GPU en GLSL — vrai temps réel (30–60 fps) |
| **TP33** | Vulkan + Hardware RTX | Vulkan, extensions `VK_KHR_ray_tracing_pipeline`, BLAS/TLAS, GLSL `rayPayloadEXT` | Ray tracing hardware (RTX/RDNA2), même paradigme que Lumen/UE5 |

---

## Vue macro — Ce que chaque phase apporte

```
TP1–4   → C++ de base : types, classes, opérateurs
TP5–8   → POO : héritage, templates, STL, mémoire
TP9–11  → Production : I/O, threads, move semantics
TP12    → C++ Moderne : lambdas, optional, structured bindings
TP12bis → Architecture : CMake, namespaces, séparation du code
TP13–14 → Algorithmes : Möller-Trumbore, BVH O(log n)
TP15–20 → Rendu physique : camera, textures, PBR, éclairage direct
TP21–28 → Performance : SIMD, allocateurs, GPU CUDA, media
TP29–31 → Interactivité : fenêtre, progressive rendering, denoising
TP32–33 → Temps réel : compute shaders, Vulkan RTX hardware
```

---

## Phase 7 — GI temps réel état de l'art (TP34–38)

À ce stade le ray tracer tourne en temps réel sur GPU. Ces TPs implémentent les algorithmes
qu'utilisent Unreal Engine 5 Lumen, Cyberpunk 2077, Alan Wake 2.

| TP | Titre | C++ / Algo | Feature graphique |
|---|---|---|---|
| **TP34** | ReSTIR DI | Reservoir sampling, weighted reservoir sampling, maths probabilistes | Éclairage direct réutilisé spatialement et temporellement — ombres douces à 1 spp |
| **TP35** | ReSTIR GI | Extension de TP34 à l'indirect, path replay, temporal reuse avec shift mapping | Illumination globale en temps réel sans irradiance probes — même qualité qu'offline |
| **TP36** | Hybrid rendering | Architecture pipeline hybride (G-buffer rasterisé + ray tracing sélectif) | Réflexions ray-tracées sur fond rasterisé — approche PS5/Series X |
| **TP37** | Radiance Cascades | Structure de données cascade + interpolation bilinéaire | GI en temps réel sans probe placement manuel — style Lumen/Path Traced UE5 |
| **TP38** | Atmosphère physique | Intégration de Rayleigh + Mie scattering, LUT pré-calculées | Ciel dynamique physiquement correct (couleur au coucher du soleil, halo lunaire) |

---

## Phase 8 — Algorithmes offline de référence (TP39–43)

Algorithmes théoriquement importants, utilisés en film/VFX (Pixar, Weta, ILM).
Moins "temps réel", mais comprendre ça permet de comprendre POURQUOI ReSTIR marche.

| TP | Titre | C++ / Algo | Feature graphique |
|---|---|---|---|
| **TP39** | BDPT | Connexion bidirectionnelle de chemins, MIS à N stratégies, graphe de transport | Caustiques et éclairage indirect difficile (scène de Cornell box parfaite) |
| **TP40** | Photon Mapping | kd-tree des photons, gathering, density estimation, progressive PM | Caustiques nettes sous l'eau, foyer de loupe — impossible en path tracing pur |
| **TP41** | Rendu spectral | Remplacement RGB par N longueurs d'onde, dispersion dans le verre | Arc-en-ciel, prisme, verre coloré physiquement correct, pas de métamérisme |
| **TP42** | Participating media avancé | Équation de transfert radiatif, hétérogène, OpenVDB lecture | Fumée/feu avec VDB (même format que Houdini), nuages volumétriques |
| **TP43** | Neural denoising / DLSS-like | Intro au ML en C++ (ONNX Runtime), inference sur CPU/GPU | Denoiser basé sur un réseau de neurones pré-entraîné — qualité DLSS sur ton renderer |

---

## Phase 9 — Vers un moteur de jeu (TP44–50)

Le ray tracer est maintenant un vrai renderer. Ces TPs construisent les couches autour
pour en faire un moteur interactif complet — le vrai objectif final.

| TP | Titre | C++ / Archi | Feature moteur |
|---|---|---|---|
| **TP44** | Scene editor — Dear ImGui | Intégration ImGui, docking, fenêtres flottantes, bindings C++ | Éditeur graphique : sélectionner objets, modifier matériaux, déplacer lights en live |
| **TP45** | Asset pipeline — GLTF/Assimp | `CMake FetchContent`, parsers tiers, sérialisation, cache d'assets | Import GLTF 2.0 complet : maillages, matériaux PBR, squelettes, animations |
| **TP46** | ECS — Entity Component System | Archetype storage, `std::tuple`, type erasure, `entt` ou impl from scratch | Architecture données-orientées : 10× plus de cache hits, base de tout moteur moderne |
| **TP47** | Physics — Rigid body | Intégration Jolt Physics (C++), collision shapes, raycasts physiques | Physique rigide : boîtes qui tombent, contraintes, triggers — terrain jouable |
| **TP48** | Audio + Input | SDL_Mixer ou OpenAL, `std::chrono` input polling, rebinding | Son 3D positionnel, musique, gestion complète du clavier/souris/gamepad |
| **TP49** | Scripting Lua | Intégration sol2/Lua 5.4, binding C++↔Lua, hot-reload de scripts | Logique de jeu scriptée sans recompiler — caméras, comportements, cinématiques |
| **TP50** | Projet final — Mini moteur | Tout ce qui précède assemblé | Un jeu jouable avec ton renderer RTX, ton ECS, ta physique, ton scripting |

---

## Vue macro complète

```
TP1–4    → C++ de base : types, classes, opérateurs
TP5–8    → POO : héritage, templates, STL, mémoire
TP9–11   → Production : I/O, threads, move semantics
TP12     → C++ Moderne : lambdas, optional, structured bindings
TP12bis  → Architecture : CMake, namespaces, séparation du code
TP13–14  → Algorithmes : Möller-Trumbore, BVH O(log n)
TP15–20  → Rendu physique : camera, textures, PBR, éclairage direct
TP21–28  → Performance : SIMD, allocateurs, GPU CUDA, media
TP29–31  → Interactivité : fenêtre, progressive rendering, denoising
TP32–33  → Temps réel : compute shaders, Vulkan RTX hardware
TP34–38  → GI état de l'art : ReSTIR, radiance cascades, atmosphère
TP39–43  → Offline de référence : BDPT, photon mapping, spectral, neural
TP44–50  → Moteur de jeu : ECS, physics, audio, scripting, éditeur
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
| STL | vector, map, optional, function, variant, span, ranges |
| Concurrence | thread, mutex, atomic, future, condition_variable, thread pool |
| Build | g++, flags, CMake, `find_package`, ExternalProject |
| C++17 | structured bindings, `if constexpr`, `std::optional`, `std::variant` |
| C++20 | concepts, ranges, `auto` param |
| C++23/26 | `std::execution`, modules, reflection |
| Performance | SIMD intrinsics, benchmarking, memory pools, ECS archetype storage |
| FFI | interop C/C++, CUDA, OpenGL, Vulkan, Lua, ONNX Runtime, Jolt, Assimp |
| Maths | algèbre linéaire, probabilités, transport de lumière, intégration Monte Carlo |
