#pragma once
#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "core/vec3.h"

namespace rt {

inline Vec3d rgbe_to_float(uint8_t r, uint8_t g, uint8_t b, uint8_t e) {
    if (e == 0) return Vec3d(0, 0, 0);
    double f = std::ldexp(1.0 / 256.0, (int)e - 128);
    return Vec3d((r + 0.5) * f, (g + 0.5) * f, (b + 0.5) * f);
}


class EnvMap {
    std::vector<float> pixels_;
    int width_  = 0;
    int height_ = 0;

    Vec3d pixel_at(int x, int y) const {
        x = ((x % width_) + width_) % width_;
        y = std::clamp(y, 0, height_ - 1);
        int idx = (y * width_ + x) * 3;
        return Vec3d(pixels_[idx], pixels_[idx+1], pixels_[idx+2]);
    }

    static void read_bytes(std::ifstream& f, uint8_t* buf, int n, const char* ctx) {
        if (!f.read(reinterpret_cast<char*>(buf), n))
            throw std::runtime_error(std::string("HDR: unexpected EOF in ") + ctx);
    }

    static void decode_new_rle(std::ifstream& f, int width,
                                std::vector<uint8_t>& out)
    {
        for (int ch = 0; ch < 4; ++ch) {
            int col = 0;
            while (col < width) {
                uint8_t code;
                read_bytes(f, &code, 1, "new-RLE code");
                if (code > 128) {
                    int count = std::min(code - 128, width - col);
                    uint8_t val;
                    read_bytes(f, &val, 1, "new-RLE run value");
                    for (int k = 0; k < count; ++k)
                        out[(col + k) * 4 + ch] = val;
                    col += count;
                } else if (code > 0) {
                    int count = std::min((int)code, width - col);
                    for (int k = 0; k < count; ++k) {
                        read_bytes(f, &out[(col + k) * 4 + ch], 1, "new-RLE literal");
                    }
                    for (int k = count; k < (int)code; ++k) {
                        uint8_t dummy;
                        read_bytes(f, &dummy, 1, "new-RLE discard");
                    }
                    col += count;
                }
            }
        }
    }

    static void decode_old_format(std::ifstream& f, int width,
                                  const uint8_t first[4],
                                  std::vector<uint8_t>& out)
    {
        uint8_t prev[4] = {0, 0, 0, 0};
        int scale = 1;
        int col   = 0;

        auto process = [&](const uint8_t* pix) {
            if (pix[0] == 1 && pix[1] == 1 && pix[2] == 1) {
                int count = std::min((int)pix[3] * scale, width - col);
                for (int k = 0; k < count; ++k, ++col) {
                    out[col * 4 + 0] = prev[0];
                    out[col * 4 + 1] = prev[1];
                    out[col * 4 + 2] = prev[2];
                    out[col * 4 + 3] = prev[3];
                }
                scale <<= 8;
            } else {
                out[col * 4 + 0] = pix[0];  out[col * 4 + 1] = pix[1];
                out[col * 4 + 2] = pix[2];  out[col * 4 + 3] = pix[3];
                prev[0] = pix[0];  prev[1] = pix[1];
                prev[2] = pix[2];  prev[3] = pix[3];
                scale = 1;
                ++col;
            }
        };

        process(first);
        while (col < width) {
            uint8_t pix[4];
            read_bytes(f, pix, 4, "old-format pixel");
            process(pix);
        }
    }

public:
    EnvMap() = default;
    EnvMap(std::vector<float> data, int w, int h)
        : pixels_(std::move(data)), width_(w), height_(h) {}

    static EnvMap load_hdr(const std::string& filename) {
        std::ifstream f(filename, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open HDR file: " + filename);

        std::string line;
        bool valid = false;
        while (std::getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line == "#?RADIANCE" || line == "#?RGBE") { valid = true; continue; }
            if (line.empty()) break;
        }
        if (!valid)
            throw std::runtime_error("Not a valid Radiance HDR file: " + filename);

        while (std::getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty()) break;
        }
        int width = 0, height = 0;
        {
            std::istringstream ss(line);
            std::string sy, sx;
            ss >> sy >> height >> sx >> width;
        }
        if (width <= 0 || height <= 0)
            throw std::runtime_error("HDR: invalid resolution line: '" + line + "'");

        std::vector<float> pixels(width * height * 3, 0.0f);
        std::vector<uint8_t> scanline(width * 4);

        for (int row = 0; row < height; ++row) {
            uint8_t hdr4[4];
            read_bytes(f, hdr4, 4, "scanline header");

            bool is_new_rle = (hdr4[0] == 2 && hdr4[1] == 2 && (hdr4[2] & 0x80) == 0);

            if (is_new_rle) {
                int w = (hdr4[2] << 8) | hdr4[3];
                if (w != width)
                    throw std::runtime_error("HDR: new-RLE scanline width mismatch");
                std::fill(scanline.begin(), scanline.end(), 0u);
                decode_new_rle(f, width, scanline);
            } else {
                std::fill(scanline.begin(), scanline.end(), 0u);
                decode_old_format(f, width, hdr4, scanline);
            }

            for (int col = 0; col < width; ++col) {
                Vec3d c = rgbe_to_float(
                    scanline[col*4+0], scanline[col*4+1],
                    scanline[col*4+2], scanline[col*4+3]);
                int idx = (row * width + col) * 3;
                pixels[idx+0] = (float)c.x;
                pixels[idx+1] = (float)c.y;
                pixels[idx+2] = (float)c.z;
            }
        }

        return EnvMap(std::move(pixels), width, height);
    }

    static std::pair<double,double> dir_to_uv(const Vec3d& dir) {
        double phi = std::atan2(dir.z, dir.x);
        double u   = phi / (2.0 * pi) + 0.5;
        u -= std::floor(u);
        double theta = std::acos(std::clamp(dir.y, -1.0, 1.0));
        double v     = theta / pi;
        return {u, v};
    }

    static Vec3d uv_to_dir(double u, double v) {
        double phi   = (u - 0.5) * 2.0 * pi;
        double theta = v * pi;
        double st    = std::sin(theta);
        return Vec3d(st * std::cos(phi), std::cos(theta), st * std::sin(phi));
    }

    Vec3d sample(double u, double v) const {
        if (pixels_.empty()) return Vec3d(0, 0, 0);
        u -= std::floor(u);
        v  = std::clamp(v, 0.0, 1.0);
        double px = u * (width_  - 1);
        double py = v * (height_ - 1);
        int x0 = (int)px,  y0 = (int)py;
        int x1 = std::min(x0 + 1, width_  - 1);
        int y1 = std::min(y0 + 1, height_ - 1);
        double fx = px - x0, fy = py - y0;
        return pixel_at(x0, y0) * ((1-fx)*(1-fy))
             + pixel_at(x1, y0) * (   fx *(1-fy))
             + pixel_at(x0, y1) * ((1-fx)*   fy )
             + pixel_at(x1, y1) * (   fx *   fy );
    }

    Vec3d background(const Vec3d& dir) const {
        if (pixels_.empty()) return Vec3d(0, 0, 0);
        auto [u, v] = dir_to_uv(dir);
        return sample(u, v);
    }

    int  width()  const { return width_;  }
    int  height() const { return height_; }
    bool empty()  const { return pixels_.empty(); }
};


struct EnvSample {
    Vec3d  direction;
    double pdf;
    Vec3d  radiance;
};

class EnvMapSampler {
    std::vector<float> marginal_cdf_;
    std::vector<float> conditional_cdf_;
    std::vector<float> marginal_pdf_;
    std::vector<float> conditional_pdf_;
    int width_  = 0;
    int height_ = 0;
    const EnvMap& env_;

    static double luminance(const Vec3d& c) {
        return 0.2126 * c.x + 0.7152 * c.y + 0.0722 * c.z;
    }

public:
    explicit EnvMapSampler(const EnvMap& env) : env_(env) {
        width_  = env.width();
        height_ = env.height();

        conditional_pdf_.resize(height_ * width_,    0.0f);
        conditional_cdf_.resize(height_ * (width_+1), 0.0f);
        marginal_pdf_.resize(height_, 0.0f);
        marginal_cdf_.resize(height_ + 1, 0.0f);

        std::vector<float> row_sums(height_, 0.0f);

        for (int y = 0; y < height_; ++y) {
            double theta  = (y + 0.5) / height_ * pi;
            double sin_w  = std::sin(theta);
            float  row_sum = 0.0f;

            for (int x = 0; x < width_; ++x) {
                double u   = (width_  > 1) ? (double)x / (width_  - 1) : 0.5;
                double v   = (height_ > 1) ? (double)y / (height_ - 1) : 0.5;
                double lum = std::max(0.0, luminance(env.sample(u, v)));
                float  w   = (float)(lum * sin_w);
                conditional_pdf_[y * width_ + x] = w;
                row_sum += w;
            }
            row_sums[y] = row_sum;

            conditional_cdf_[y * (width_+1)] = 0.0f;
            for (int x = 0; x < width_; ++x) {
                float p = (row_sum > 0.0f)
                    ? conditional_pdf_[y * width_ + x] / row_sum
                    : 1.0f / width_;
                conditional_pdf_[y * width_ + x] = p;
                conditional_cdf_[y * (width_+1) + x + 1] =
                    conditional_cdf_[y * (width_+1) + x] + p;
            }
            conditional_cdf_[y * (width_+1) + width_] = 1.0f;
        }

        float total = 0.0f;
        for (float r : row_sums) total += r;

        marginal_cdf_[0] = 0.0f;
        for (int y = 0; y < height_; ++y) {
            marginal_pdf_[y] = (total > 0.0f)
                ? row_sums[y] / total
                : 1.0f / height_;
            marginal_cdf_[y + 1] = marginal_cdf_[y] + marginal_pdf_[y];
        }
        marginal_cdf_[height_] = 1.0f;
    }

    EnvSample sample(double xi1, double xi2) const {
        auto it_row = std::upper_bound(marginal_cdf_.begin(), marginal_cdf_.end(),
                                       (float)xi1);
        int row = (int)std::distance(marginal_cdf_.begin(), it_row) - 1;
        row = std::clamp(row, 0, height_ - 1);

        auto row_begin = conditional_cdf_.begin() + row * (width_ + 1);
        auto row_end   = row_begin + (width_ + 1);
        auto it_col    = std::upper_bound(row_begin, row_end, (float)xi2);
        int col = (int)std::distance(row_begin, it_col) - 1;
        col = std::clamp(col, 0, width_ - 1);

        double u   = (col + 0.5) / width_;
        double v   = (row + 0.5) / height_;
        Vec3d  dir = EnvMap::uv_to_dir(u, v);
        Vec3d  rad = env_.sample(u, v);

        return EnvSample{dir, pdf(dir), rad};
    }

    double pdf(const Vec3d& dir) const {
        auto [u, v] = EnvMap::dir_to_uv(dir);
        int row = std::clamp((int)(v * height_), 0, height_ - 1);
        int col = std::clamp((int)(u * width_),  0, width_  - 1);
        double sin_theta = std::sin(v * pi);
        if (sin_theta < 1e-10) return 0.0;
        return (double)marginal_pdf_[row]
             * (double)conditional_pdf_[row * width_ + col]
             * (double)(width_ * height_)
             / (2.0 * pi * pi * sin_theta);
    }
};

}
