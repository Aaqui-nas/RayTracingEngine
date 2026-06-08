# TP4 — Surcharge d'opérateurs

## Objectif final
Rendre `Vec3` pleinement utilisable avec les opérateurs mathématiques (`+`, `-`, `*`, `/`, produit scalaire, produit vectoriel), et mettre à jour `Ray::at` pour en bénéficier.

---

## 1. Surcharge d'opérateurs

C++ permet de redéfinir le comportement des opérateurs pour tes classes :

```cpp
Vec3 operator+(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
```

On peut aussi les définir comme méthodes membres :

```cpp
class Vec3 {
public:
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
};
```

Règle pratique : les opérateurs **binaires symétriques** (`+`, `-`) se définissent souvent en dehors de la classe (fonctions libres) pour que les deux opérandes soient traités de façon égale.

---

## 2. Opérateurs à implémenter

| Opérateur | Signature | Description |
|---|---|---|
| `+` | `Vec3 operator+(const Vec3&, const Vec3&)` | Addition composante par composante |
| `-` | `Vec3 operator-(const Vec3&, const Vec3&)` | Soustraction |
| `*` | `Vec3 operator*(const Vec3&, double)` | Multiplication par scalaire |
| `*` | `Vec3 operator*(double, const Vec3&)` | Idem, ordre inversé |
| `/` | `Vec3 operator/(const Vec3&, double)` | Division par scalaire |
| `-` (unaire) | `Vec3 operator-(const Vec3&)` | Négation |
| `dot` | `double dot(const Vec3&, const Vec3&)` | Produit scalaire |
| `cross` | `Vec3 cross(const Vec3&, const Vec3&)` | Produit vectoriel |
| `<<` | `std::ostream& operator<<(std::ostream&, const Vec3&)` | Affichage |

---

## 3. Opérateurs d'affectation composés

```cpp
Vec3& operator+=(Vec3& a, const Vec3& b) {
    a.x += b.x; a.y += b.y; a.z += b.z;
    return a;
}
```

---

## Exercice

Dans `vec3.h` (ou un nouveau `vec3.cpp`) :

1. Implémente tous les opérateurs du tableau ci-dessus
2. Implémente `+=`, `-=`, `*=`, `/=`
3. Mets à jour `Ray::at(t)` pour utiliser les opérateurs : `return origin + direction * t;`
4. Mets à jour `main.cpp` pour utiliser `<<` pour afficher les couleurs

### Question à se poser
Pourquoi définit-on `operator*(double, const Vec3&)` en plus de `operator*(const Vec3&, double)` ? Que se passe-t-il si on écrit `2.0 * v` avec seulement le premier défini ?

---

## Critères de réussite
- [x] Tous les opérateurs sont implémentés
- [x] `Ray::at(t)` utilise les opérateurs
- [x] `dot(a, b)` et `cross(a, b)` fonctionnent
- [x] `std::cout << vec3` affiche `x y z`
- [x] `main.cpp` compile et génère la même image
