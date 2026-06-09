# TP8 — STL : vector, algorithm, boucles range-based

## Objectif final
Exploiter la STL pour gérer la liste d'objets et les pixels, et ajouter des utilitaires de traitement d'image (luminosité, gamma correction).

---

## 1. `std::vector`

Tableau dynamique, la structure la plus utilisée en C++ :

```cpp
#include <vector>

std::vector<int> v;
v.push_back(42);
v.push_back(7);
std::cout << v.size();   // 2
std::cout << v[0];       // 42
```

---

## 2. Boucle range-based

```cpp
for (int x : v) {
    std::cout << x << " ";
}

// Avec référence pour éviter la copie :
for (const auto& x : v) {
    std::cout << x << " ";
}
```

---

## 3. Algorithmes STL

```cpp
#include <algorithm>

std::sort(v.begin(), v.end());

auto it = std::find(v.begin(), v.end(), 42);
if (it != v.end()) { /* trouvé */ }

// Prédicat lambda (aperçu, détails au TP12) :
auto it2 = std::find_if(v.begin(), v.end(), [](int x){ return x > 10; });
```

---

## 4. Gamma correction

Les écrans affichent les couleurs dans un espace gamma non linéaire. Pour un rendu correct :

```
couleur_affichée = pow(couleur_linéaire, 1/gamma)   // gamma ≈ 2.2
```

Simplifié : `sqrt(valeur)` est une bonne approximation (gamma = 2.0).

---

## Exercice

1. Refactorise `Scene` pour utiliser un range-based for dans `hit()`
2. Crée une structure `Pixel { int r, g, b; }` et stocke tous les pixels dans un `std::vector<Pixel>` avant d'écrire le fichier
3. Applique une gamma correction (sqrt) sur chaque composante avant l'écriture
4. Ajoute une fonction `clamp(double v, double min, double max)` et utilise-la pour garantir [0, 255]
5. Crée une fonction `sort_by_distance` qui trie un `vector<double>` de distances (exercice pour manipuler `std::sort`)

### Question à se poser
Pourquoi stocker tous les pixels avant d'écrire plutôt qu'écrire directement dans la boucle ? (Indice : pense au TP10 sur le multithreading.)

---

## Critères de réussite
- [x] Range-based for utilisé dans `Scene::hit()`
- [x] Les pixels sont stockés dans un `vector<Pixel>`
- [x] La gamma correction est appliquée
- [x] `clamp` est utilisée
- [x] `std::sort` est utilisé au moins une fois
