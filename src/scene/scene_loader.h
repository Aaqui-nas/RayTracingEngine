#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <map>
#include "scene/scene.h"
#include "geometry/sphere.h"
#include "geometry/plane.h"
#include "scene/obj_loader.h"
#include "camera/camera.h"
#include "materials/env_map.h"
#include "geometry/procedural.h"
#include "scene/transform_node.h"
#include "geometry/box.h"
#include "geometry/quadric.h"
#include "geometry/csg.h"

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
            std::cerr << "Matériau inconnu: " << name << "\n";
            return materials.at("default");
        };

        // Récursif : parse un shape ou un transform wrappant un shape
        std::function<std::shared_ptr<Shape>(const std::string&, std::istringstream&)> parse_shape;
        parse_shape = [&](const std::string& type, std::istringstream& ss) -> std::shared_ptr<Shape> {

            // ── Transforms (récursifs) ─────────────────────────────────────────
            if (type == "translate") {
                double dx, dy, dz; std::string inner; ss >> dx >> dy >> dz >> inner;
                auto s = parse_shape(inner, ss);
                return s ? make_translate(s, dx, dy, dz) : nullptr;
            }
            if (type == "rotate_x") {
                double a; std::string inner; ss >> a >> inner;
                auto s = parse_shape(inner, ss);
                return s ? make_rotate_x(s, a) : nullptr;
            }
            if (type == "rotate_y") {
                double a; std::string inner; ss >> a >> inner;
                auto s = parse_shape(inner, ss);
                return s ? make_rotate_y(s, a) : nullptr;
            }
            if (type == "rotate_z") {
                double a; std::string inner; ss >> a >> inner;
                auto s = parse_shape(inner, ss);
                return s ? make_rotate_z(s, a) : nullptr;
            }
            if (type == "scale") {
                double sx, sy, sz; std::string inner; ss >> sx >> sy >> sz >> inner;
                auto s = parse_shape(inner, ss);
                return s ? make_scale(s, sx, sy, sz) : nullptr;
            }

            // ── Primitives ────────────────────────────────────────────────────
            if (type == "sphere") {
                double x, y, z, r; std::string mat;
                ss >> x >> y >> z >> r >> mat;
                return std::make_shared<Sphere>(Vec3d(x,y,z), r, get_material(mat));
            }
            if (type == "plane") {
                double x1, y1, z1, x2, y2, z2; std::string mat;
                ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> mat;
                return std::make_shared<Plane>(Vec3d(x1,y1,z1), Vec3d(x2,y2,z2), get_material(mat));
            }
            if (type == "mesh") {
                std::string path, mat; ss >> path >> mat;
                return load_obj(path, get_material(mat));
            }
            if (type == "torus") {
                double x, y, z, R, r; int nu, nv; std::string mat;
                ss >> x >> y >> z >> R >> r >> nu >> nv >> mat;
                auto mesh_ptr = std::make_shared<Mesh>(generate_torus(R, r, nu, nv, get_material(mat)));
                return make_translate(mesh_ptr, x, y, z);
            }
            if (type == "sphere_mesh") {
                double x, y, z, radius; int nu, nv; std::string mat;
                ss >> x >> y >> z >> radius >> nu >> nv >> mat;
                auto mesh_ptr = std::make_shared<Mesh>(generate_sphere_mesh(radius, nu, nv, get_material(mat)));
                return make_translate(mesh_ptr, x, y, z);
            }
            if (type == "box") {
                double x1, y1, z1, x2, y2, z2; std::string mat;
                ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> mat;
                return std::make_shared<Box>(Vec3d(x1,y1,z1), Vec3d(x2,y2,z2), get_material(mat));
            }
            if (type == "cylinder") {
                double radius, height; std::string mat;
                ss >> radius >> height >> mat;
                return std::make_shared<Cylinder>(radius, height, get_material(mat));
            }
            if (type == "cone") {
                double radius, height; std::string mat;
                ss >> radius >> height >> mat;
                return std::make_shared<Cone>(radius, height, get_material(mat));
            }
            if (type == "csg_union" || type == "csg_inter" || type == "csg_diff") {
                auto parse_csg = [&](auto factory) -> std::shared_ptr<Shape> {
                    std::string t1; ss >> t1; auto a = parse_shape(t1, ss);
                    std::string t2; ss >> t2; auto b = parse_shape(t2, ss);
                    return (a && b) ? factory(a, b) : nullptr;
                };
                if (type == "csg_union") return parse_csg(csg_union);
                if (type == "csg_inter") return parse_csg(csg_inter);
                return parse_csg(csg_diff);
            }
            if (type == "terrain") {
                int N, M; double cell_size, height_scale, noise_scale, noise_amplitude; std::string mat;
                ss >> N >> M >> cell_size >> height_scale >> noise_scale >> noise_amplitude >> mat;
                std::vector<std::vector<double>> heights(N, std::vector<double>(M, 0.0));
                apply_noise(heights, noise_scale, noise_amplitude);
                return std::make_shared<Mesh>(generate_terrain(heights, cell_size, height_scale, get_material(mat)));
            }
            return nullptr;
        };

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream ss(line);
            std::string type;
            ss >> type;

            if (type == "camera") {
                double x1, y1, z1, x2, y2, z2, x3, y3, z3, fov, aperture, focus;
                ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3 >> fov >> aperture >> focus;
                camera = Camera(Vec3d(x1,y1,z1), Vec3d(x2,y2,z2), Vec3d(x3,y3,z3),
                                fov, aspect_ratio, aperture, focus);
            } else if (type == "envmap") {
                std::string path; ss >> path;
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
            } else {
                auto shape = parse_shape(type, ss);
                if (shape) {
                    scene.add(shape);
                } else if (!type.empty() && type != "#") {
                    std::cerr << "Type inconnu: " << type << "\n";
                }
            }
        }
        return scene;
    }

}
