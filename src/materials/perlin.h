#pragma once
#include <array>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "core/vec3.h"

namespace rt {

    class PerlinNoise {
        static constexpr int N = 256;

        std::array<int, N>    perm;
        std::array<Vec3d, N>  grad;

        static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
        static double lerp(double a, double b, double t) { return a + t * (b - a); }

    public:
        PerlinNoise() {
            std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<double> dist(-1.0, 1.0);

            for (int i = 0; i < N; ++i) {
                perm[i] = i;
                Vec3d g;
                do { g = Vec3d(dist(rng), dist(rng), dist(rng)); } while (g.length() < 1e-6);
                grad[i] = g.normalized();
            }
            std::shuffle(perm.begin(), perm.end(), rng);
        }

        double noise(const Vec3d& p) const {
            int xi = (int)std::floor(p.x);
            int yi = (int)std::floor(p.y);
            int zi = (int)std::floor(p.z);

            double xf = p.x - xi;
            double yf = p.y - yi;
            double zf = p.z - zi;

            double u = fade(xf), v = fade(yf), w = fade(zf);

            auto gradient_at = [&](int x, int y, int z) -> const Vec3d& {
                return grad[perm[(x & 255)] ^ perm[(y & 255)] ^ perm[(z & 255)]];
            };

            double dots[2][2][2];
            for (int dx = 0; dx < 2; ++dx)
                for (int dy = 0; dy < 2; ++dy)
                    for (int dz = 0; dz < 2; ++dz)
                        dots[dx][dy][dz] = dot(
                            gradient_at(xi+dx, yi+dy, zi+dz),
                            Vec3d(xf-dx, yf-dy, zf-dz)
                        );

            double x00 = lerp(dots[0][0][0], dots[1][0][0], u);
            double x10 = lerp(dots[0][1][0], dots[1][1][0], u);
            double x01 = lerp(dots[0][0][1], dots[1][0][1], u);
            double x11 = lerp(dots[0][1][1], dots[1][1][1], u);

            return lerp(lerp(x00, x10, v), lerp(x01, x11, v), w);
        }

        double turbulence(Vec3d p, int octaves = 7) const {
            double sum    = 0.0;
            double weight = 1.0;
            for (int i = 0; i < octaves; ++i) {
                sum    += weight * std::abs(noise(p));
                weight *= 0.5;
                p      *= 2.0;
            }
            return sum / (2.0 - std::pow(2.0, 1 - octaves));
        }
    };

}
