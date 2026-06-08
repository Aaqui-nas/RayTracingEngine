# TP6 — Templates

## Objectif final
Rendre `Vec3` générique avec les templates, pour qu'il puisse travailler aussi bien avec `float` qu'avec `double` ou `int`.

---

## 1. Templates de fonctions

Un template permet d'écrire du code générique, instancié par le compilateur pour chaque type utilisé :

```cpp
template<typename T>
T max_val(T a, T b) {
    return (a > b) ? a : b;
}

max_val(3, 5);       // T = int
max_val(3.0, 5.0);   // T = double
```

---

## 2. Templates de classes

```cpp
template<typename T>
class Vec3 {
public:
    T x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
};

Vec3<double> v(1.0, 2.0, 3.0);
Vec3<float>  f(1.0f, 2.0f, 3.0f);
Vec3<int>    i(1, 2, 3);
```

---

## 3. Contrainte importante

**Les templates doivent être entièrement définis dans le `.h`**, pas séparés en `.h`/`.cpp`. Le compilateur a besoin du code complet pour instancier le template.

---

## 4. Aliases pratiques

```cpp
using Vec3d = Vec3<double>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;
```

---

## Exercice

1. Transforme `Vec3` en `Vec3<T>` dans `vec3.h`
2. Déplace toutes les implémentations dans le `.h` (si elles étaient dans un `.cpp`)
3. Ajoute les aliases `Vec3d`, `Vec3f`, `Vec3i`
4. Mets à jour `ray.h` pour utiliser `Vec3d` (ou `Vec3<double>`)
5. Vérifie que `main.cpp` compile toujours

### Bonus
Ajoute un template de fonction `clamp<T>(T val, T min, T max)` qui limite une valeur entre min et max, et utilise-la pour garantir que les composantes de couleur restent dans [0, 255].

### Question à se poser
Pourquoi `length()` retourne `double` même dans un `Vec3<int>` ? Est-ce correct ? Comment le corriger ?

---

## Critères de réussite
- [x] `Vec3<T>` fonctionne avec `double`, `float`, et `int`
- [x] Les aliases `Vec3d`, `Vec3f`, `Vec3i` existent
- [x] `Ray` utilise `Vec3d`
- [x] Le projet compile et génère la même image
