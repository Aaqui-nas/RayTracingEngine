# Projet fil rouge — Ray Tracer

Ray tracer from scratch en C++ pur, sans librairie externe.
Sortie : images au format PPM.

## Pourquoi ce projet
Couvre un large spectre du C++ tout en étant visuellement motivant et pertinent pour le développement de moteurs de jeu.

## Organisation du projet (post-refactor)

- **Un seul répertoire de code** à la racine de `LearnCPP/` — pas de dossiers `TPn/` séparés.
- **Code évolue en place** au fil des TPs : on modifie les fichiers existants, on en crée de nouveaux.
- **Sujets** dans `subjects/TP1.md` → `subjects/TP14.md` (+ suite) — documents autonomes sans skeleton files.
- **Version courante** : après TP12 (lambdas, matériaux, anti-aliasing, émission, multithreading).

## Fichiers actifs (racine du projet)
```
main.cpp, vec3.h, ray.h, shape.h, sphere.h, plane.h,
scene.h, scene_loader.h, scene.txt
```

## État du code après TP12
- Anti-aliasing (multi-sampling)
- Matériaux : diffuse (lambertian), métal (fuzz), verre (Schlick + Snell), checker, verre teinté
- Source de lumière émissive (`emission` sur Shape)
- Multithreading (std::thread par bande de lignes)
- Chargement de scène depuis `scene.txt`
- Scène "Crystal Hall" : miroirs en face à face, sphère de verre, plan en verre, source de lumière

## Progression des TPs

| TP | Concept C++ | Feature graphique |
|---|---|---|
| TP1 | Variables, types, I/O | Image PPM dégradée |
| TP2 | Fonctions, références | Premiers calculs de couleur |
| TP3 | Structs/Classes | Vec3, Ray |
| TP4 | Surcharge d'opérateurs | Vec3 arithmétique complète |
| TP5 | Héritage, polymorphisme | Shape → Sphere, Plane |
| TP6 | Templates | Vec3<T> générique |
| TP7 | Smart pointers | Scène avec shared_ptr |
| TP8 | STL | Liste d'objets, utilitaires |
| TP9 | File I/O, parsing | Scene loader (scene.txt) |
| TP10 | Multithreading | Rendu parallèle |
| TP11 | Move semantics | Optimisation des copies |
| TP12 | Lambdas, C++17, optional | Matériaux, anti-aliasing, verre, émission |
| TP12bis | CMake, namespaces, .h/.cpp | Classe Camera, build_materials() |
| TP13 | Möller-Trumbore, parsing OBJ | Triangle, Mesh, import OBJ |
| TP14 | AABB, BVH, operator[] | Accélération O(log n) |

## Roadmap complète
Voir `memory/roadmap_complet.md` pour les TP15 → TP50 détaillés.
