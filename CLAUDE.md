# LearnCPP — Contexte du projet

## Mémoire du projet
- [Profil utilisateur](memory/user_profile.md)
- [Rôle de Claude](memory/feedback_teaching_role.md)
- [Projet fil rouge : Ray Tracer](memory/project_raytracer.md)

## Organisation du projet

### Structure des fichiers
```
LearnCPP/
├── CLAUDE.md
├── CMakeLists.txt        ← build principal + tests + benchmarks
├── memory/               ← mémoire persistante du projet
│   ├── roadmap_complet.md
│   └── ...
├── subjects/             ← TOUS les sujets de TP (TP1.md → TP60.md)
│   ├── TP1.md ... TP14.md, TP12bis.md
│   └── (nouveaux sujets créés au fil des TPs)
├── tests/                ← tests unitaires (Catch2)
│   ├── test_vec3.cpp, test_ray.cpp
│   ├── test_shapes.cpp, test_scene.cpp
│   └── test_materials.cpp
├── benchmarks/           ← microbenchmarks (Google Benchmark)
│   ├── bench_vec3.cpp
│   └── bench_hit.cpp
├── main.cpp              ← code C++ actif (version courante)
├── vec3.h, ray.h, shape.h, sphere.h, plane.h
├── scene.h, scene_loader.h
├── scene.txt
└── (nouveaux fichiers créés au fil des TPs)
```

### Principe de fonctionnement
- **Un seul répertoire de code** : tout le code C++ est à la racine — pas de dossiers TP séparés.
- **Le code évolue en place** : chaque TP modifie ou enrichit les fichiers existants.
- **Les sujets** dans `subjects/` sont les guides pédagogiques autonomes.
- **Version actuelle** : après TP12 (lambdas, matériaux, anti-aliasing, émission, multithreading).

## Résumé rapide
- **Utilisateur** : apprend C++ from scratch, objectif moteurs de jeu
- **Projet** : ray tracer from scratch, ~50 TPs progressifs
- **Rôle de Claude** : prof/guide uniquement — indices et explications, jamais le code à la place de l'utilisateur
- **Roadmap** : voir `memory/roadmap_complet.md` pour les 50 TPs détaillés
