# TP7 — Smart Pointers et Gestion Mémoire

## Objectif final
Construire une `Scene` qui contient une liste d'objets via `shared_ptr`, et tester l'intersection avec tous les objets pour retenir le plus proche.

---

## 1. Le problème de la gestion manuelle

```cpp
Shape* s = new Sphere(...);
// ... si une exception est levée ici, la mémoire est perdue
delete s;  // on doit penser à libérer !
```

---

## 2. `unique_ptr` — propriété exclusive

```cpp
#include <memory>

auto s = std::make_unique<Sphere>(Vec3(0,0,-1), 0.5);
// libéré automatiquement quand s sort de portée
// on ne peut pas copier s, seulement le déplacer
```

---

## 3. `shared_ptr` — propriété partagée

```cpp
auto s = std::make_shared<Sphere>(Vec3(0,0,-1), 0.5);
auto s2 = s;  // deux propriétaires, compteur de références = 2
// libéré quand le dernier propriétaire est détruit
```

---

## 4. Quand utiliser lequel ?

- `unique_ptr` : par défaut, quand un seul propriétaire suffit (plus efficace)
- `shared_ptr` : quand l'objet est partagé entre plusieurs endroits
- Dans une scène de ray tracer, les objets peuvent être partagés → `shared_ptr`

---

## Exercice

Crée `scene.h` :

### `scene.h`
Classe `Scene` avec :
- `std::vector<std::shared_ptr<Shape>> objects`
- Méthode `void add(std::shared_ptr<Shape> shape)`
- Méthode `bool hit(const Ray& ray, double tmin, double tmax, double& t, int& index) const`
  - Teste tous les objets, retient le plus proche (t minimum)
  - Retourne `true` si au moins un objet est touché

### `main.cpp`
- Crée une scène avec :
  - Une sphère rouge en (0, 0, -1), rayon 0.5
  - Une sphère verte en (-1, 0, -1), rayon 0.5
  - Un plan gris à y = -0.5, normal (0, 1, 0)
- Pour chaque rayon, utilise `scene.hit()` et colorie selon l'objet touché

### Question à se poser
Pourquoi `Scene` stocke `shared_ptr<Shape>` et pas `Shape*` ou `Shape` directement ? Que se passerait-il avec `Shape` direct (hint : polymorphisme et slicing) ?

---

## Critères de réussite
- [x] `Scene` est utilisée avec `shared_ptr`
- [x] `make_shared` est utilisé (jamais `new` seul)
- [x] Les trois objets apparaissent dans l'image
- [x] Le plus proche est bien retenu quand deux objets se chevauchent
- [x] Aucune fuite mémoire (pas de `new`/`delete` nu)
