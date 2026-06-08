# TP3 — Structs, Classes, Constructeurs

## Objectif final
Créer les briques de base du ray tracer : `Vec3` (vecteur 3D) et `Ray` (rayon = origine + direction).

---

## 1. Les structs

Une `struct` regroupe des données liées :

```cpp
struct Point {
    double x;
    double y;
    double z;
};

Point p;
p.x = 1.0;
```

---

## 2. Les classes

Une `class` est identique à une `struct`, sauf que les membres sont **privés par défaut** :

```cpp
class Vec3 {
public:
    double x, y, z;

    // constructeur
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
};
```

Le `: x(x), y(y), z(z)` est la **liste d'initialisation** — c'est la façon idiomatique d'initialiser les membres.

---

## 3. Méthodes membres

Une classe peut avoir des fonctions qui opèrent sur ses données :

```cpp
class Vec3 {
public:
    double x, y, z;
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
};
```

Le `const` après le nom de la méthode signifie qu'elle ne modifie pas l'objet.

---

## 4. Encapsulation : public / private

```cpp
class Vec3 {
public:
    Vec3(double x, double y, double z);
    double length() const;

private:
    double x_, y_, z_;  // convention : _ pour les membres privés
};
```

---

## Exercice

Crée deux fichiers : `vec3.h` et `ray.h`.

### `vec3.h`
Définis une classe `Vec3` avec :
- Membres publics `double x, y, z`
- Un constructeur `Vec3(double x, double y, double z)`
- Un constructeur par défaut `Vec3()` qui initialise à (0, 0, 0)
- Une méthode `double length() const` qui retourne la norme du vecteur
- Une méthode `Vec3 normalized() const` qui retourne le vecteur de longueur 1

### `ray.h`
Définis une classe `Ray` avec :
- Membres : `Vec3 origin` et `Vec3 direction`
- Un constructeur `Ray(Vec3 origin, Vec3 direction)`
- Une méthode `Vec3 at(double t) const` qui retourne `origin + direction * t`
  (tu n'as pas encore les opérateurs — utilise les coordonnées x, y, z directement)

### `main.cpp`
- Crée un rayon dont l'origine est (0, 0, 0) et la direction est (0, 1, 0)
- Affiche `ray.at(2.5)` : doit donner (0, 2.5, 0)
- Génère l'image du TP2 en utilisant `Vec3` pour stocker la couleur

### Question à se poser
Pourquoi `length()` et `normalized()` sont-elles marquées `const` ? Que se passerait-il si on les appelait sur un `const Vec3` sans ce mot-clé ?

---

## Critères de réussite
- [x] `vec3.h` et `ray.h` compilent sans erreur
- [x] `Vec3` a deux constructeurs
- [x] `Ray::at(t)` retourne le bon point
- [x] `main.cpp` utilise `Vec3` pour les couleurs
- [x] Aucune variable globale
