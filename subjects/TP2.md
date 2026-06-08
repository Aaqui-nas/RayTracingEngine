# TP2 — Fonctions, références, const

## Objectif final
Refactoriser le code du TP1 en introduisant des fonctions, et ajouter une première fonction de calcul de couleur basée sur la direction d'un rayon fictif.

---

## 1. Les fonctions

Une fonction regroupe du code réutilisable :

```cpp
// Déclaration (prototype)
int add(int a, int b);

// Définition
int add(int a, int b) {
    return a + b;
}
```

Le type de retour `void` signifie que la fonction ne retourne rien.

---

## 2. Passage par valeur vs par référence

Par défaut, C++ copie les arguments :

```cpp
void double_val(int x) {
    x = x * 2;  // modifie la copie, pas l'original
}
```

Pour modifier l'original, on passe par **référence** avec `&` :

```cpp
void double_val(int& x) {
    x = x * 2;  // modifie l'original
}
```

Pour lire sans copier (efficace sur les gros objets), on utilise `const&` :

```cpp
void print_val(const int& x) {
    std::cout << x;  // lecture seule, pas de copie
}
```

---

## 3. `const`

`const` empêche la modification d'une variable :

```cpp
const int MAX = 255;
MAX = 100;  // erreur de compilation
```

Bonne pratique : tout ce qui ne doit pas changer doit être `const`.

---

## 4. Fonctions utilitaires pour le ray tracer

Dans un ray tracer, on sépare la logique en fonctions claires :
- `write_color` : écrit un pixel sur stdout
- `ray_color` : calcule la couleur d'un rayon selon sa direction

---

## Exercice

À partir du code du TP1, refactorise et étends-le :

1. Crée une fonction `write_color(int r, int g, int b)` qui affiche `r g b ` sur stdout
2. Crée une fonction `write_header(int width, int height)` qui affiche l'en-tête PPM
3. Crée une fonction `ray_color(double dir_y)` qui retourne un `int` entre 0 et 255 :
   - Si `dir_y > 0.0` → retourne une valeur proportionnelle à `dir_y` (entre 0 et 255)
   - Sinon → retourne 0
4. Dans les boucles, utilise ces fonctions. Pour `dir_y`, passe la valeur normalisée de `i` : `(double)i / height`

### Question à se poser
Pourquoi `write_color` n'a pas besoin de retourner de valeur ? Dans quel cas utiliserait-on `const int&` plutôt que `int` comme paramètre ?

---

## Critères de réussite
- [x] Le programme compile sans erreur
- [x] Les fonctions `write_color`, `write_header`, `ray_color` existent et sont utilisées
- [x] `write_header` et `write_color` sont `void`
- [x] Au moins un paramètre `const` est utilisé
- [x] L'image générée est identique (visuellement) à celle du TP1, avec le canal bleu remplacé par `ray_color`
