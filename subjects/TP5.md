# TP5 — Héritage, Polymorphisme, Virtual

## Objectif final
Créer une hiérarchie de formes géométriques (`Shape` → `Sphere`, `Plane`) et implémenter un premier ray tracer fonctionnel : envoyer un rayon, tester l'intersection, colorier le pixel.

---

## 1. Héritage

Une classe peut hériter d'une autre et en réutiliser le code :

```cpp
class Animal {
public:
    std::string name;
    Animal(std::string name) : name(name) {}
    void breathe() { std::cout << "breathing\n"; }
};

class Dog : public Animal {
public:
    Dog(std::string name) : Animal(name) {}
    void bark() { std::cout << "woof\n"; }
};
```

---

## 2. Fonctions virtuelles

Sans `virtual`, l'appel dépend du **type statique** (celui de la variable) :

```cpp
Animal* a = new Dog("Rex");
a->speak();  // appelle Animal::speak, pas Dog::speak !
```

Avec `virtual`, l'appel dépend du **type dynamique** (l'objet réel) :

```cpp
class Animal {
public:
    virtual void speak() { std::cout << "...\n"; }
};
class Dog : public Animal {
public:
    void speak() override { std::cout << "woof\n"; }
};

Animal* a = new Dog("Rex");
a->speak();  // appelle Dog::speak ✓
```

---

## 3. Classes abstraites et fonctions purement virtuelles

Une fonction `= 0` doit être redéfinie par les classes filles :

```cpp
class Shape {
public:
    virtual bool hit(const Ray& ray, double& t) const = 0;  // pure virtual
    virtual ~Shape() {}  // toujours un destructeur virtuel !
};
```

Une classe avec au moins une fonction pure virtuelle est **abstraite** — on ne peut pas l'instancier directement.

---

## 4. `override`

Le mot-clé `override` vérifie que tu redéfinis bien une fonction virtuelle de la classe parente. Toujours l'utiliser.

---

## Exercice

Crée `shape.h`, `sphere.h`, `plane.h`.

### `shape.h`
Classe abstraite `Shape` avec :
- Méthode pure virtuelle `bool hit(const Ray& ray, double tmin, double tmax, double& t) const`
- Destructeur virtuel

### `sphere.h`
Classe `Sphere : public Shape` avec :
- `Vec3 center` et `double radius`
- Constructeur
- `hit()` implémenté : intersection rayon-sphère
  - Formule : `t = (-b ± sqrt(b²-4ac)) / 2a` où `a = dot(d,d)`, `b = 2*dot(d, oc)`, `c = dot(oc,oc) - r²`, `oc = ray.origin - center`, `d = ray.direction`
  - Retourne `true` si discriminant ≥ 0 et t dans [tmin, tmax]

### `plane.h`
Classe `Plane : public Shape` avec :
- `Vec3 point` (un point sur le plan) et `Vec3 normal`
- `hit()` : intersection rayon-plan
  - Formule : `t = dot(normal, point - ray.origin) / dot(normal, ray.direction)`
  - Retourne `true` si dénominateur ≠ 0 et t dans [tmin, tmax]

### `main.cpp`
- Place une sphère de rayon 0.5 en (0, 0, -1)
- Caméra à l'origine, plan image à z = -1, viewport 2x2
- Pour chaque pixel, envoie un rayon et colorie :
  - Sphère touchée → rouge
  - Sinon → dégradé bleu/blanc selon la direction y du rayon

### Question à se poser
Pourquoi le destructeur de `Shape` doit-il être `virtual` ? Que se passe-t-il si on `delete` un `Shape*` pointant vers une `Sphere` sans destructeur virtuel ?

---

## Critères de réussite
- [x] `Shape` est abstraite (non instanciable)
- [x] `Sphere` et `Plane` implémentent `hit()` avec `override`
- [x] La sphère apparaît en rouge dans l'image
- [x] Le fond est un dégradé bleu/blanc
- [x] Aucun `new`/`delete` manuel (utilise des objets sur la pile pour l'instant)
