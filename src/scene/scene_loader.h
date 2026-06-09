#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include "scene/scene.h"
#include "geometry/sphere.h"
#include "geometry/plane.h"
#include "scene/obj_loader.h"
#include "camera/camera.h"

namespace rt {

    inline Scene load_scene(const std::string& filename,
                            const std::map<std::string, Material>& materials,
                            Camera& camera,
                            double aspect_ratio)
    {
        Scene scene;
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Erreur: fichier introuvable: " << filename << "\n";
            return scene;
        }

        auto get_material = [&](const std::string& name) -> Material {
            auto it = materials.find(name);
            if (it != materials.end()) return it->second;
            std::cerr << "Matériau inconnu: " << name << ", utilisation du matériau par défaut\n";
            return materials.at("default");
        };

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream ss(line);
            std::string type;
            ss >> type;

            if (type == "sphere") {
                double x, y, z, r;
                std::string mat;
                ss >> x >> y >> z >> r >> mat;
                scene.add(std::make_shared<Sphere>(Vec3d(x,y,z), r, get_material(mat)));
            } else if (type == "plane") {
                double x1, y1, z1, x2, y2, z2;
                std::string mat;
                ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> mat;
                scene.add(std::make_shared<Plane>(Vec3d(x1,y1,z1), Vec3d(x2,y2,z2), get_material(mat)));
            } else if (type == "light") {
                double x, y, z, r, er, eg, eb;
                ss >> x >> y >> z >> r >> er >> eg >> eb;
                auto s = std::make_shared<Sphere>(Vec3d(x,y,z), r, get_material("__black__"));
                s->emission = Vec3d(er, eg, eb);
                scene.add(std::move(s));
            } else if (type == "mesh") {
                std::string path, mat;
                ss >> path >> mat;
                scene.add(load_obj(path, get_material(mat)));
            } else if (type == "camera") {
                double x1, y1, z1, x2, y2, z2, x3, y3, z3, fov, aperture, focus;
                ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3 >> fov >> aperture >> focus;
                camera = Camera(Vec3d(x1,y1,z1), Vec3d(x2,y2,z2), Vec3d(x3,y3,z3),
                                fov, aspect_ratio, aperture, focus);
            } else if (!type.empty() && type != "#") {
                std::cerr << "Type inconnu: " << type << "\n";
            }
        }
        return scene;
    }

}
