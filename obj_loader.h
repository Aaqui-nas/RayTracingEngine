#pragma once
#include "mesh.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

namespace rt {
    inline std::shared_ptr<Mesh> load_obj(const std::string& path, Material mat) {
        auto mesh = std::make_shared<Mesh>(mat);
        std::vector<Vec3d> vertices;

        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string type;
            ss >> type;


            if (type == "v") {
                double x, y, z;
                ss >> x >> y >> z;
                vertices.push_back(Vec3d(x,y,z));
            } else if (type == "f") {
                int x, y, z;
                ss >> x >> y >> z;
                mesh->add(Triangle(vertices[x-1],vertices[y-1],vertices[z-1], mat));
            }
        }
        return mesh;
    }
}
