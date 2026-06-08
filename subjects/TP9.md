# TP9 — File I/O et Parsing

## Objectif final
Charger une scène depuis un fichier texte, pour ne plus avoir à recompiler quand on change la scène.

---

## 1. Écrire dans un fichier

```cpp
#include <fstream>

std::ofstream file("output.txt");
file << "Hello, file!\n";
file.close();  // ou laisse le destructeur le faire
```

---

## 2. Lire depuis un fichier

```cpp
std::ifstream file("scene.txt");
if (!file.is_open()) {
    std::cerr << "Erreur: fichier introuvable\n";
    return 1;
}

std::string line;
while (std::getline(file, line)) {
    std::cout << line << "\n";
}
```

---

## 3. Parser une ligne

```cpp
#include <sstream>

std::string line = "sphere 0 0 -1 0.5";
std::istringstream ss(line);

std::string type;
double x, y, z, r;
ss >> type >> x >> y >> z >> r;
```

---

## 4. Format de scène proposé

```
# commentaire
sphere 0 0 -1 0.5
sphere -1 0 -1 0.5
plane 0 -0.5 0  0 1 0
```

Chaque ligne commence par le type d'objet suivi de ses paramètres.

---

## Exercice

1. Crée une fonction `Scene load_scene(const std::string& filename)` dans un fichier `scene_loader.h`
2. Elle lit le fichier ligne par ligne et crée les objets correspondants avec `make_shared`
3. Les lignes commençant par `#` ou vides sont ignorées
4. Crée le fichier `scene.txt` avec au moins 3 obj
sphere  0.0  0.0 -1.0  0.5
ets
5. Modifie `main.cpp` pour charger la scène depuis `scene.txt` au lieu de la créer dans le code
6. Si le fichier est absent, affiche un message d'erreur sur `std::cerr` et quitte avec `return 1`

### Bonus
Ajoute la couleur dans le fichier de scène :
```
sphere 0 0 -1 0.5  255 0 0
```
Et stocke-la dans les objets (nécessite d'ajouter un champ couleur à `Shape`).

### Question à se poser
Pourquoi utilise-t-on `std::cerr` pour les erreurs plutôt que `std::cout` ? Rappel : `./raytracer > image.ppm`...

---

## Critères de réussite
- [ ] `load_scene()` parse correctement le fichier
- [ ] Les lignes vides et commentaires sont ignorés
- [ ] Les types inconnus affichent un warning sur `cerr`
- [ ] `main.cpp` n'a plus d'objets codés en dur
- [ ] Le programme gère proprement le fichier manquant
