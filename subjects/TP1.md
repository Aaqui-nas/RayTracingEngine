# TP1 — Premiers pas : variables, types, I/O

## Objectif final
Générer ta première image en écrivant un programme C++ qui affiche un dégradé de couleurs au format PPM.

Résultat attendu :

![gradient](https://i.imgur.com/placeholder.png)
*(un dégradé rouge → noir en horizontal, vert → noir en vertical)*

---

## 1. Compiler et exécuter

Tout programme C++ doit être **compilé** avant d'être exécuté. Pour ce projet, on utilise `g++`.

```bash
# Compiler
g++ -o raytracer main.cpp

# Exécuter et rediriger la sortie vers un fichier image
./raytracer > image.ppm
```

Pour visualiser le fichier `.ppm` généré, utilise [ce viewer en ligne](https://www.cs.rhodes.edu/welshc/COMP141_F16/ppmReader.html) ou installe `eog` / `feh` sur Linux.

---

## 2. Les types de base en C++

En C++, chaque variable a un **type** déclaré explicitement :

```
int     → entier (ex: 42, -7)
double  → nombre décimal (ex: 3.14)
float   → décimal moins précis, plus léger (ex: 3.14f)
bool    → vrai/faux (true / false)
char    → un caractère (ex: 'a')
```

Déclaration et initialisation :
```
type nom = valeur;
```

---

## 3. Les opérateurs

Les opérateurs arithmétiques classiques : `+`, `-`, `*`, `/`

Point important : en C++, la division entre deux entiers donne un entier.
```
5 / 2    → 2   (division entière)
5.0 / 2  → 2.5 (division décimale)
```

---

## 4. Afficher avec `cout`

`cout` permet d'afficher du texte et des valeurs dans le terminal :

```cpp
#include <iostream>

std::cout << "Hello" << std::endl;
std::cout << 42 << std::endl;
std::cout << "La valeur est : " << maVariable << std::endl;
```

`std::cerr` fonctionne de la même façon mais affiche sur la sortie d'erreur (utile pour les logs de progression, pour ne pas polluer l'image).

---

## 5. Les boucles `for`

```cpp
for (int i = 0; i < 10; i++) {
    // exécuté 10 fois, i vaut 0, 1, 2, ... 9
}
```

---

## 6. Le format PPM

Le format PPM est le plus simple qui existe pour une image. C'est du texte pur :

```
P3
<largeur> <hauteur>
<valeur_max>
r g b  r g b  r g b ...
```

Exemple pour une image 2x2 :
```
P3
2 2
255
255 0 0   0 255 0
0 0 255   255 255 0
```

Chaque pixel est défini par trois valeurs **r g b** entre 0 et 255.
Les pixels sont écrits ligne par ligne, de haut en bas, de gauche à droite.

---

## Exercice

Génère une image de **256 x 256 pixels** avec le dégradé suivant :
- La composante **rouge** augmente de gauche à droite (colonne 0 → noir, colonne 255 → rouge pur)
- La composante **verte** augmente de haut en bas (ligne 0 → noir, ligne 255 → vert pur)
- La composante **bleue** est fixée à 64

### Étapes suggérées

1. Affiche l'en-tête PPM (`P3`, dimensions, valeur max)
2. Parcours chaque ligne avec une boucle `for`
3. Pour chaque ligne, parcours chaque colonne avec une boucle `for` imbriquée
4. Calcule les valeurs r, g, b du pixel en fonction de la position
5. Affiche les valeurs séparées par des espaces

### Question à se poser
Comment convertir une position (entre 0 et 255) en une valeur de couleur (entre 0.0 et 1.0), puis en un entier entre 0 et 255 ?

---

## Critères de réussite
- [x] Le programme compile sans erreur
- [x] `./raytracer > image.ppm` génère un fichier lisible
- [x] L'image affiche bien un dégradé rouge/vert avec du bleu fixe
- [x] Le code utilise des variables typées explicitement (pas `auto` pour l'instant)
