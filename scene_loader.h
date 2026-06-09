#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <functional>
#include "scene.h"
#include "sphere.h"
#include "plane.h"

namespace rt {
    Scene load_scene(const std::string& filename, const std::map<std::string, Material>& materials) {
        Scene scene;
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Erreur: fichier introuvable\n";
            return scene;
        }

        auto get_material = [&](const std::string& name) {
            auto it = materials.find(name);
            if (it != materials.end())
                return it->second;

            std::cerr << "Matériau inconnu: " << name
                    << ", utilisation du matériau par défaut\n";

            return materials.at("default");
        };

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);

            std::string type, material;
            ss >> type;
            if (line.empty() || line[0] == '#') {}
            else if (type == "sphere") {
                double x, y, z, r;
                ss >> x >> y >> z >> r >> material;
                scene.add(std::make_shared<Sphere>(Vec3d(x,y,z),r,get_material(material)));
            } else if (type == "plane")
            {
                double x1, y1, z1, x2, y2, z2;
                ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> material;
                scene.add(std::make_shared<Plane>(Vec3d(x1,y1,z1),Vec3d(x2,y2,z2),get_material(material)));
            } else if (type == "light") {
                double x, y, z, r, er, eg, eb;
                ss >> x >> y >> z >> r >> er >> eg >> eb;
                auto s = std::make_shared<Sphere>(Vec3d(x,y,z), r, get_material("__black__"));
                s->emission = Vec3d(er, eg, eb);
                scene.add(std::move(s));
            } else if (!type.empty() && type != "#") {
                std::cerr << "Type inconnu: " << type << "\n";
            }

        }
        return scene;
    }
}
