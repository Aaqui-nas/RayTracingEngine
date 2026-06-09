#pragma once
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include "core/vec3.h"
#include "materials/perlin.h"

namespace rt {

    class Texture {
    public:
        virtual Vec3d sample(double u, double v, const Vec3d& point) const = 0;
        virtual ~Texture() {}
    };

    using TexturePtr = std::shared_ptr<Texture>;

    class SolidColor : public Texture {
    public:
        Vec3d color;
        SolidColor(Vec3d color) : color(color) {}

        Vec3d sample(double, double, const Vec3d&) const override { return color; }
    };

    class CheckerTexture : public Texture {
    public:
        Vec3d  even, odd;
        double scale;

        CheckerTexture(Vec3d even, Vec3d odd, double scale)
            : even(even), odd(odd), scale(scale) {}

        Vec3d sample(double u, double v, const Vec3d&) const override {
            bool is_even = ((int)std::floor(u * scale) + (int)std::floor(v * scale)) % 2 == 0;
            return is_even ? even : odd;
        }
    };

    class ImageTexture : public Texture {
        int width  = 0;
        int height = 0;
        std::vector<uint8_t> data;

    public:
        ImageTexture(const std::vector<uint8_t>& d, int w, int h)
            : width(w), height(h), data(d) {}

        ImageTexture(const std::string& filename) {
            std::ifstream file(filename);
            if (!file) throw std::runtime_error("Cannot open texture file: " + filename);

            std::string magic;
            file >> magic;
            if (magic != "P3") throw std::runtime_error("Only PPM P3 format is supported");

            int max_value;
            file >> width >> height >> max_value;
            if (max_value != 255) throw std::runtime_error("Only max value 255 is supported");

            data.resize(width * height * 3);
            for (size_t i = 0; i < data.size(); ++i) {
                int value;
                file >> value;
                data[i] = static_cast<uint8_t>(value);
            }
        }

        Vec3d sample(double u, double v, const Vec3d&) const override {
            if (width == 0 || height == 0) return Vec3d(255, 0, 255);
            int px = ((int)(u * width))  % width;
            int py = ((int)(v * height)) % height;
            if (px < 0) px += width;
            if (py < 0) py += height;
            int idx = (py * width + px) * 3;
            return Vec3d(data[idx], data[idx+1], data[idx+2]);
        }
    };

    class PerlinTexture : public Texture {
        PerlinNoise noise;
        Vec3d       color;
        double      scale;

    public:
        PerlinTexture(const Vec3d& color, double scale)
            : color(color), scale(scale) {}

        Vec3d sample(double, double, const Vec3d& p) const override {
            return color * noise.turbulence(p * scale);
        }
    };

}
