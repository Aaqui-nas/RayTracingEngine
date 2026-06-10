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
#include "materials/env_map.h"
#include "geometry/procedural.h"

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
            } else if (type == "mesh") {
                std::string path, mat;
                ss >> path >> mat;
                scene.add(load_obj(path, get_material(mat)));
            } else if (type == "camera") {
                double x1, y1, z1, x2, y2, z2, x3, y3, z3, fov, aperture, focus;
                ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3 >> fov >> aperture >> focus;
                camera = Camera(Vec3d(x1,y1,z1), Vec3d(x2,y2,z2), Vec3d(x3,y3,z3),
                                fov, aspect_ratio, aperture, focus);
            } else if (type == "envmap") {
                std::string path;
                ss >> path;
                try {
                    scene.env_map = std::make_shared<EnvMap>(EnvMap::load_hdr(path));
                } catch (const std::exception& e) {
                    std::cerr << "Erreur envmap: " << e.what() << "\n";
                }
            } else if (type == "light") {
                double x, y, z, r, er, eg, eb;
                ss >> x >> y >> z >> r >> er >> eg >> eb;
                auto s = std::make_shared<Sphere>(Vec3d(x,y,z), r, get_material("__black__"));
                s->emission = Vec3d(er, eg, eb);
                scene.add(std::move(s));
                scene.lights.add(make_sphere_light(Vec3d(x,y,z), r, Vec3d(er,eg,eb)));
            } else if (type == "torus") {
                double x, y, z, R, r;
                int nu, nv;
                std::string mat;

                ss >> x >> y >> z >> R >> r >> nu >> nv >> mat;

                auto mesh = generate_torus(R, r, nu, nv, get_material(mat));
                mesh.translate(Vec3d(x, y, z));
                scene.add(std::make_shared<Mesh>(std::move(mesh)));

            } else if (type == "sphere_mesh") {
                double x, y, z, radius;
                int nu, nv;
                std::string mat;

                ss >> x >> y >> z >> radius >> nu >> nv >> mat;

                auto mesh = generate_sphere_mesh(radius, nu, nv, get_material(mat));
                mesh.translate(Vec3d(x, y, z));
                scene.add(std::make_shared<Mesh>(std::move(mesh)));

            } else if (type == "terrain") {
                int N, M;
                double cell_size, height_scale, noise_scale, noise_amplitude;
                std::string mat;

                ss >> N >> M
                >> cell_size
                >> height_scale
                >> noise_scale
                >> noise_amplitude
                >> mat;

                std::vector<std::vector<double>> heights(
                    N, std::vector<double>(M, 0.0));

                apply_noise(heights, noise_scale, noise_amplitude);

                auto mesh = generate_terrain(
                    heights,
                    cell_size,
                    height_scale,
                    get_material(mat));

                scene.add(std::make_shared<Mesh>(std::move(mesh)));
            }
            else if  (!type.empty() && type != "#") {
                std::cerr << "Type inconnu: " << type << "\n";
            }
        }
        return scene;
    }

}
