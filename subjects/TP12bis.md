# TP12bis — Refactoring : CMake, Namespaces, .h/.cpp, Classe Camera

## Pourquoi refactorer ?

Le code de TP12 fonctionne — mais à mesure que les projets grossissent, une bonne organisation devient indispensable :

- **Lisibilité** : des fichiers de 500 lignes deviennent vite illisibles ; séparer les responsabilités facilite la navigation.
- **Réutilisabilité** : un `Vec3` dans son propre fichier peut être réutilisé dans n'importe quel projet sans tout embarquer.
- **Évolutivité** : CMake gère les dépendances et la compilation pour toi ; tu n'as plus à te souvenir de chaque flag g++.

---

## Section 1 — CMake

CMake est un outil de build multiplateforme : il génère les fichiers Makefile (ou les projets Visual Studio, Xcode…) à partir d'un fichier `CMakeLists.txt`.

### Les directives essentielles

```cmake
# Version minimale de CMake requise
cmake_minimum_required(VERSION 3.15)

# Nom du projet, version, langages utilisés
project(RayTracer VERSION 1.0 LANGUAGES CXX)

# Standard C++ à utiliser (obligatoire = on ne compile pas si non supporté)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cherche la bibliothèque Threads du système (pthreads sur Linux)
find_package(Threads REQUIRED)

# Déclare l'exécutable et ses fichiers sources
add_executable(raytracer main.cpp materials.cpp)

# Lie l'exécutable avec Threads (équivaut à -pthread)
target_link_libraries(raytracer PRIVATE Threads::Threads)

# Options de compilation supplémentaires
target_compile_options(raytracer PRIVATE -O2 -Wall)
```

### Commandes de build

```bash
mkdir build && cd build
cmake ..     # configure le projet (génère le Makefile)
make         # compile
./raytracer  # exécute (produit du PPM sur stdout)
```

> Astuce : `cmake .. -DCMAKE_BUILD_TYPE=Release` active les optimisations maximales.

---

## Section 2 — Namespaces

Un **namespace** est un espace de noms : il évite les collisions entre identifiants de bibliothèques différentes. Si une lib externe déclare aussi une classe `Vec3`, le compilateur ne saura pas laquelle choisir — sauf si elles sont dans des namespaces distincts.

### Syntaxe de base

```cpp
// Dans vec3.h — on englobe tout le contenu
namespace rt {

template<typename T>
class Vec3 { ... };

using Vec3d = Vec3<double>;
// etc.

} // namespace rt
```

Pour utiliser `Vec3d` depuis l'extérieur :

```cpp
// Option 1 : qualifier explicitement (à préférer dans les .h)
rt::Vec3d v(1, 0, 0);

// Option 2 : importer dans la portée courante (à réserver aux .cpp)
using namespace rt;
Vec3d v(1, 0, 0);
```

### Pourquoi ne PAS mettre `using namespace rt;` dans un .h ?

Un `.h` est inclus par de nombreux fichiers. Si tu y mets `using namespace rt;`, tu imposes cette importation à **tous** les fichiers qui incluent ce header — ce qui peut créer des conflits invisibles et difficiles à déboguer.

**Règle d'or** : `using namespace` uniquement dans les `.cpp`, jamais dans les `.h`.

### Avant / Après — exemple avec vec3.h

**Avant (TP12)** :
```cpp
// vec3.h
template<typename T>
class Vec3 { ... };
using Vec3d = Vec3<double>;
```

**Après (TP12bis)** :
```cpp
// vec3.h
namespace rt {
    template<typename T>
    class Vec3 { ... };
    using Vec3d = Vec3<double>;
} // namespace rt
```

```cpp
// main.cpp
#include "vec3.h"
using namespace rt;   // une seule ligne, et on peut écrire Vec3d partout
Vec3d v(1, 0, 0);
```

---

## Section 3 — Séparation .h / .cpp

### La règle ODR (One Definition Rule)

En C++, une **déclaration** peut apparaître plusieurs fois ; une **définition** ne peut apparaître **qu'une seule fois** dans tout le programme.

- `.h` → **déclarations** : signatures de fonctions, définitions de classes, `inline` functions, templates
- `.cpp` → **définitions** : corps des fonctions non-inline, variables globales

Si tu définis `Material make_lambertian(Vec3d color) { ... }` dans un `.h` inclus par deux `.cpp`, le linker se retrouve avec deux définitions de la même fonction → **erreur de compilation**.

### Solution : déclarer dans le .h, définir dans le .cpp

```cpp
// materials.h — déclarations uniquement
#pragma once
#include "shape.h"
#include <map>
#include <string>

namespace rt {

Material make_lambertian(Vec3d color);
Material make_metal(Vec3d color, double fuzz);
Material make_glass(double ior);
Material make_tinted_glass(Vec3d tint, double ior);
Material make_checker(double scale = 2.0);
Material make_black();
std::map<std::string, Material> build_materials();

} // namespace rt
```

```cpp
// materials.cpp — définitions
#include "materials.h"
using namespace rt;

Material make_lambertian(Vec3d color) {
    return [color](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
        Vec3d dir = rec.normal + random_unit_vector();
        if (dot(dir, dir) < 1e-8) dir = rec.normal;
        return Scatter{color, Ray(rec.point, dir)};
    };
}
```

### Exception : le mot-clé `inline`

Pour les fonctions très courtes définies dans un `.h` (comme `load_scene` dans `scene_loader.h`), on peut utiliser `inline` pour autoriser plusieurs définitions identiques :

```cpp
// Autorisé dans un .h si toutes les définitions sont identiques
inline Scene load_scene(...) { ... }
```

Les **templates** et les **méthodes de classe définies dans le corps de la classe** sont implicitement `inline`.

---

## Section 4 — Classe Camera

### Pourquoi encapsuler la caméra ?

Dans TP12, la configuration de la caméra est dispersée en 4 variables globales dans `main.cpp` :

```cpp
const Vec3d origin(0, 0, 0);
const Vec3d horizontal(4.0, 0, 0);
const Vec3d vertical(0, 2.25, 0);
const Vec3d lower_left_corner = origin - horizontal/2 - vertical/2 - Vec3d(0,0,1);
```

Ces variables sont utilisées dans `render_rows` par capture implicite. C'est fragile : si on veut changer la caméra (angle de vue, position, etc.), il faut modifier plusieurs endroits.

**L'encapsulation** regroupe ces données et leur logique dans une classe :

```cpp
namespace rt {

class Camera {
public:
    Vec3d origin;
    Vec3d lower_left_corner;
    Vec3d horizontal;
    Vec3d vertical;

    Camera();
    Camera(double vfov_deg, double aspect_ratio);

    Ray get_ray(double u, double v) const;
};

} // namespace rt
```

### Formules du constructeur paramétrique

Pour un champ de vision vertical `vfov_deg` (en degrés) et un ratio `aspect_ratio` :

```
theta          = vfov_deg * π / 180
h              = tan(theta / 2)
viewport_height = 2.0 * h
viewport_width  = aspect_ratio * viewport_height

origin         = Vec3d(0, 0, 0)
horizontal     = Vec3d(viewport_width, 0, 0)
vertical       = Vec3d(0, viewport_height, 0)
lower_left_corner = origin - horizontal/2 - vertical/2 - Vec3d(0, 0, 1)
```

Le constructeur par défaut `Camera()` correspond à `Camera(50.0, 16.0/9.0)` et reproduit **exactement** le setup de TP12.

### La méthode `get_ray`

```cpp
Ray get_ray(double u, double v) const {
    return Ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
}
```

---

## Exercices

### Étape 1 — Créer CMakeLists.txt

Crée un fichier `CMakeLists.txt` à la racine du projet avec :

1. `cmake_minimum_required(VERSION 3.15)`
2. `project(RayTracer VERSION 1.0 LANGUAGES CXX)`
3. `set(CMAKE_CXX_STANDARD 17)` et `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
4. `find_package(Threads REQUIRED)`
5. `add_executable(raytracer main.cpp materials.cpp)`
6. `target_link_libraries(raytracer PRIVATE Threads::Threads)`
7. `target_compile_options(raytracer PRIVATE -O2 -Wall)`

Teste immédiatement (même si `materials.cpp` n'existe pas encore, crée-le vide pour que CMake compile) :

```bash
mkdir build && cd build
cmake ..
make
```

---

### Étape 2 — Ajouter `namespace rt { }` dans tous les headers

Dans chacun de ces fichiers, entoure **tout le contenu** (après les `#pragma once` et `#include`) dans `namespace rt { ... }` :

- `vec3.h`
- `ray.h`
- `shape.h`
- `sphere.h`
- `plane.h`
- `scene.h`
- `scene_loader.h`

**Attention** : dans `vec3.h`, les `using Vec3d = ...` doivent rester **à l'intérieur** du namespace.

Recompile après chaque fichier modifié.

---

### Étape 3 — Ajouter `using namespace rt;` dans main.cpp

En haut de `main.cpp`, sous les `#include`, ajoute :

```cpp
using namespace rt;
```

Recompile. Le programme doit produire la même image qu'avant.

---

### Étape 4 — Créer camera.h avec la classe Camera

Crée `camera.h` avec la classe décrite en Section 4 :

```cpp
#pragma once
#include "ray.h"
#include <cmath>

namespace rt {

class Camera {
public:
    Vec3d origin;
    Vec3d lower_left_corner;
    Vec3d horizontal;
    Vec3d vertical;

    Camera();
    Camera(double vfov_deg, double aspect_ratio);

    Ray get_ray(double u, double v) const;
};

} // namespace rt
```

Implémente les méthodes directement dans le .h (elles sont courtes). Utilise les formules de la Section 4.

**Test** : dans `main.cpp`, remplace les 4 variables globales `origin`, `horizontal`, `vertical`, `lower_left_corner` par :
```cpp
Camera cam;
```

Dans `render_rows`, passe `cam` en paramètre et remplace :
```cpp
Ray ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
```
par :
```cpp
Ray ray = cam.get_ray(u, v);
```

---

### Étape 5 — Créer materials.h et materials.cpp

**Crée `materials.h`** avec uniquement les déclarations :

```cpp
#pragma once
#include "shape.h"
#include <map>
#include <string>

namespace rt {

Material make_lambertian(Vec3d color);
Material make_metal(Vec3d color, double fuzz);
Material make_glass(double ior);
Material make_tinted_glass(Vec3d tint, double ior);
Material make_checker(double scale = 2.0);
Material make_black();
std::map<std::string, Material> build_materials();

} // namespace rt
```

**Crée `materials.cpp`** avec les implémentations :

1. Déplace les helpers depuis `main.cpp` dans un bloc `namespace { }` anonyme (hors namespace rt) :
   - `rng`, `rng_dist`, `rng_dist01` (thread_local)
   - `random_in_unit_sphere()`, `random_unit_vector()`
   - `reflect()`, `refract()`, `schlick()`

2. Dans `namespace rt`, implémente chaque fonction factory en copiant les lambdas depuis `main.cpp`.

3. Implémente `build_materials()` : elle construit et retourne la `std::map<std::string, Material>` complète avec tous les matériaux (red, green, blue, white, normal, gradient, silver, gold, copper, mirror, fuzzy_metal, glass, glass_thick, glass_blue, glass_rose, __black__, checker, default).

---

### Étape 6 — Nettoyer main.cpp

1. Supprime les 4 variables globales `origin`, `horizontal`, `vertical`, `lower_left_corner`.
2. Supprime les helpers devenus inutiles (`random_in_unit_sphere`, `reflect`, `refract`, `schlick`, `random_unit_vector`) de `main.cpp`.
3. Dans `main()`, remplace tout le bloc de déclarations de matériaux par :
   ```cpp
   auto materials = build_materials();
   ```
4. Ajoute `Camera cam;` et passe-la à `render_rows`.

Recompile. L'image produite doit être identique à celle de TP12.

---

## Critères de réussite

- [x] `CMakeLists.txt` compile sans erreur avec `cmake .. && make`
- [x] Tous les headers sont entourés de `namespace rt { }`
- [x] `main.cpp` utilise `using namespace rt;` et compile
- [x] `Camera::get_ray()` est implémentée et remplace les globales
- [x] `materials.h` contient uniquement des déclarations (pas de définitions)
- [x] `materials.cpp` contient toutes les implémentations des factories
- [x] `build_materials()` retourne la map complète
- [x] Le programme produit une image PPM identique à celle de TP12
