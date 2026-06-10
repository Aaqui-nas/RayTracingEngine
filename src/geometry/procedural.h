#pragma once
#include "geometry/mesh.h"
#include "materials/materials.h"
#include <vector>
#include <cmath>
#include <numbers>

namespace rt {

    Mesh generate_torus(double R, double r, int nu, int nv, const Material& mat) {
        constexpr double pi = std::numbers::pi;

        std::vector<Vec3d> vertices;
        vertices.reserve(static_cast<std::size_t>(nu) * nv);
        for (int i = 0; i < nu; ++i) {
            const double ui = 2 * pi * i / nu;
            const double cos_ui = std::cos(ui);
            const double sin_ui = std::sin(ui);
            for (int j = 0; j < nv; ++j) {
                const double vj   = 2 * pi * j / nv;
                const double ring = R + r * std::cos(vj);
                vertices.emplace_back(ring * cos_ui, r * std::sin(vj), ring * sin_ui);
            }
        }

        Mesh mesh(mat);
        for (int i = 0; i < nu; ++i) {
            const int i1 = (i + 1) % nu;
            for (int j = 0; j < nv; ++j) {
                const int j1 = (j + 1) % nv;
                const Vec3d& a = vertices[i  * nv + j ];
                const Vec3d& b = vertices[i1 * nv + j ];
                const Vec3d& c = vertices[i  * nv + j1];
                const Vec3d& d = vertices[i1 * nv + j1];
                mesh.add(Triangle(a, b, c, mat));
                mesh.add(Triangle(b, c, d, mat));
            }
        }
        return mesh;
    }

    Mesh generate_terrain(const std::vector<std::vector<double>>& heights,
                          double cell_size, double height_scale, const Material& mat) {
        const int N = static_cast<int>(heights.size());
        const int M = static_cast<int>(heights[0].size());

        std::vector<Vec3d> vertices;
        vertices.reserve(static_cast<std::size_t>(N) * M);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < M; ++j)
                vertices.emplace_back(i * cell_size, heights[i][j] * height_scale, j * cell_size);

        Mesh mesh(mat);
        for (int i = 0; i < N - 1; ++i) {
            for (int j = 0; j < M - 1; ++j) {
                const Vec3d& a = vertices[i       * M + j    ];
                const Vec3d& b = vertices[(i + 1) * M + j    ];
                const Vec3d& c = vertices[i       * M + j + 1];
                const Vec3d& d = vertices[(i + 1) * M + j + 1];
                mesh.add(Triangle(a, b, c, mat));
                mesh.add(Triangle(b, c, d, mat));
            }
        }
        return mesh;
    }

    Mesh generate_sphere_mesh(double radius, int nu, int nv, const Material& mat) {
        constexpr double pi = std::numbers::pi;

        std::vector<Vec3d> vertices(2 + static_cast<std::size_t>(nu - 2) * nv);
        vertices.front() = Vec3d(0, radius, 0);
        for (int i = 1; i <= nu - 2; ++i) {
            const double theta = pi * i / (nu - 1);
            const double st = std::sin(theta);
            const double ct = std::cos(theta);
            for (int j = 0; j < nv; ++j) {
                const double phi = 2 * pi * j / nv;
                vertices[1 + (i - 1) * nv + j] =
                    radius * Vec3d(st * std::cos(phi), ct, st * std::sin(phi));
            }
        }
        vertices.back() = Vec3d(0, -radius, 0);

        Mesh mesh(mat);

        for (int j = 0; j < nv; ++j)
            mesh.add(Triangle(vertices[0], vertices[1 + j], vertices[1 + (j + 1) % nv], mat));

        for (int i = 0; i < nu - 3; ++i) {
            const int row  = 1 + i       * nv;
            const int row1 = 1 + (i + 1) * nv;
            for (int j = 0; j < nv; ++j) {
                const int j1 = (j + 1) % nv;
                mesh.add(Triangle(vertices[row  + j], vertices[row1 + j], vertices[row  + j1], mat));
                mesh.add(Triangle(vertices[row1 + j], vertices[row  + j1], vertices[row1 + j1], mat));
            }
        }

        const int last = static_cast<int>(vertices.size()) - 1;
        const int base = last - nv;
        for (int j = 0; j < nv; ++j)
            mesh.add(Triangle(vertices[base + j], vertices[base + (j + 1) % nv], vertices[last], mat));

        return mesh;
    }

    std::vector<Vec3d> make_uv_grid(int nu, int nv) {
        std::vector<Vec3d> grid;
        grid.reserve(static_cast<std::size_t>(nu) * nv);
        for (int i = 0; i < nu; ++i)
            for (int j = 0; j < nv; ++j)
                grid.emplace_back(double(i) / nu, double(j) / nv, 0);
        return grid;
    }

    void apply_noise(std::vector<std::vector<double>>& heights,
                 double scale, double amplitude) {
        PerlinNoise perlin;
        const int N = static_cast<int>(heights.size());
        for (int i = 0; i < N; ++i) {
            const int M = static_cast<int>(heights[i].size());
            for (int j = 0; j < M; ++j)
                heights[i][j] += amplitude * perlin.noise(Vec3d(i * scale, j * scale, 0.0));
        }
    }

}
