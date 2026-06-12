#pragma once

#include <functional>
#include <vector>
#include <span>
#include <cmath>
#include <algorithm>
#include <limits>
#include <random>
#include "core/vec3.h"

namespace rt {
    struct PixelHDR {
        double r, g, b;
    };

    struct ImageBuffer {
        int width  = 0;
        int height = 0;
        std::vector<PixelHDR> pixels;

        ImageBuffer() = default;
        ImageBuffer(int w, int h) : width(w), height(h), pixels((size_t)w * h) {}
        ImageBuffer(int w, int h, std::vector<PixelHDR> px)
            : width(w), height(h), pixels(std::move(px)) {}

        PixelHDR& at(int x, int y) { return pixels[y * width + x]; }
        const PixelHDR& at(int x, int y) const { return pixels[y * width + x]; }
        std::span<PixelHDR> row(int y) { return {pixels.data() + y * width, (size_t)width}; }
    };

    using PostPass = std::function<void(ImageBuffer&)>;

    class PostPipeline {
        std::vector<PostPass> passes;

    public:
        PostPipeline& add(PostPass pass) {
            passes.push_back(std::move(pass));
            return *this;
        }

        void apply(ImageBuffer& buf) const {
            for (const auto& pass : passes)
                pass(buf);
        }
    };

    // white=0 (défaut) → Reinhard simple L/(1+L)
    // white>0          → Reinhard étendu L×(1+L/white²)/(1+L), white = luminance blanche scène
    inline PostPass tone_map_reinhard(double white = 0.0) {
        return [white](ImageBuffer& buffer) -> void {
            auto tm = [white](double L) -> double {
                double factor = (white > 0.0) ? (1.0 + L / (white * white)) : 1.0;
                return L * factor / (1.0 + L);
            };
            for (int i = 0; i < buffer.width; i++) {
                for (int j = 0; j < buffer.height; j++) {
                    buffer.at(i,j).r = tm(buffer.at(i,j).r);
                    buffer.at(i,j).g = tm(buffer.at(i,j).g);
                    buffer.at(i,j).b = tm(buffer.at(i,j).b);
                }
            }
        };
    }
    inline PostPass tone_map_aces() {
        return [](ImageBuffer& buffer) -> void {
            auto aces = [](double x) {
                return std::clamp((x*(2.51*x+0.03))/(x*(2.43*x+0.59)+0.14), 0.0, 1.0);
            };
            for (int i = 0; i < buffer.width; i++) {
                for (int j = 0; j < buffer.height; j++) {
                    double L_in = buffer.at(i,j).r;
                    buffer.at(i,j).r = aces(L_in);
                    L_in = buffer.at(i,j).g;
                    buffer.at(i,j).g = aces(L_in);
                    L_in = buffer.at(i,j).b;
                    buffer.at(i,j).b = aces(L_in);
                }
            }
        };
    }
    inline PostPass gamma_correct(double gamma = 2.2) {
        return [gamma](ImageBuffer& buffer) -> void {
            for (int i = 0; i < buffer.width; i++) {
                for (int j = 0; j < buffer.height; j++) {
                    double L_in = buffer.at(i,j).r;
                    buffer.at(i,j).r = pow(L_in, 1.0/gamma);
                    L_in = buffer.at(i,j).g;
                    buffer.at(i,j).g = pow(L_in, 1.0/gamma);
                    L_in = buffer.at(i,j).b;
                    buffer.at(i,j).b = pow(L_in, 1.0/gamma);
                }
            }
        };
    }

    inline std::vector<double> make_gaussian_kernel(int radius, double sigma) {
        int ksize = 2 * radius + 1;
        std::vector<double> kernel(static_cast<size_t>(ksize) * ksize);

        const double inv2s2 = 1.0 / (2.0 * sigma * sigma);

        double sum = 0.0;
        for (int ky = -radius; ky <= radius; ++ky) {
            for (int kx = -radius; kx <= radius; ++kx) {
                double v = std::exp(-(kx * kx + ky * ky) * inv2s2);
                kernel[(ky + radius) * ksize + (kx + radius)] = v;
                sum += v;
            }
        }

        for (double& v : kernel)
            v /= sum;

        return kernel;
    }

    inline ImageBuffer convolve(const ImageBuffer& src, std::span<const double> kernel, int radius) {
        int ksize = 2 * radius + 1;

        ImageBuffer dst;
        dst.width  = src.width;
        dst.height = src.height;
        dst.pixels.resize(static_cast<size_t>(src.width) * src.height);

        for (int y = 0; y < src.height; ++y) {
            for (int x = 0; x < src.width; ++x) {
                PixelHDR acc{0.0, 0.0, 0.0};

                for (int dy = -radius; dy <= radius; ++dy) {
                    for (int dx = -radius; dx <= radius; ++dx) {
                        int sx = std::clamp(x + dx, 0, src.width  - 1);
                        int sy = std::clamp(y + dy, 0, src.height - 1);

                        double w = kernel[(dy + radius) * ksize + (dx + radius)];
                        const PixelHDR& p = src.at(sx, sy);
                        acc.r += w * p.r;
                        acc.g += w * p.g;
                        acc.b += w * p.b;
                    }
                }

                dst.at(x, y) = acc;
            }
        }

        return dst;
    }

    inline PixelHDR  operator*(const PixelHDR& p, double s) {
        return {p.r * s, p.g * s, p.b * s};
    }
    inline PixelHDR& operator+=(PixelHDR& a, const PixelHDR& b) {
        a.r += b.r; a.g += b.g; a.b += b.b;
        return a;
    }

    inline std::vector<double> make_gaussian_kernel_1d(int radius, double sigma) {
        int ksize = 2 * radius + 1;
        std::vector<double> k(ksize);

        const double inv2s2 = 1.0 / (2.0 * sigma * sigma);
        double sum = 0.0;
        for (int i = -radius; i <= radius; ++i) {
            double v = std::exp(-(i * i) * inv2s2);
            k[i + radius] = v;
            sum += v;
        }
        for (double& v : k) v /= sum;
        return k;
    }

    inline ImageBuffer extract_bright(const ImageBuffer& src, double threshold) {
        ImageBuffer dst;
        dst.width  = src.width;
        dst.height = src.height;
        dst.pixels.resize(static_cast<size_t>(src.width) * src.height);

        for (int y = 0; y < src.height; ++y) {
            for (int x = 0; x < src.width; ++x) {
                const PixelHDR& p = src.at(x, y);
                double lum = 0.2126 * p.r + 0.7152 * p.g + 0.0722 * p.b;
                dst.at(x, y) = (lum > threshold) ? p : PixelHDR{0.0, 0.0, 0.0};
            }
        }
        return dst;
    }

    inline ImageBuffer blur_horizontal(const ImageBuffer& src, int radius, double sigma = 0.0) {
        if (sigma <= 0.0) sigma = radius / 3.0;   // ±3σ ≈ rayon
        auto k = make_gaussian_kernel_1d(radius, sigma);

        ImageBuffer dst;
        dst.width  = src.width;
        dst.height = src.height;
        dst.pixels.resize(static_cast<size_t>(src.width) * src.height);

        for (int y = 0; y < src.height; ++y) {
            for (int x = 0; x < src.width; ++x) {
                PixelHDR acc{0.0, 0.0, 0.0};
                for (int dx = -radius; dx <= radius; ++dx) {
                    int sx = std::clamp(x + dx, 0, src.width - 1);  // clamp aux bords
                    double w = k[dx + radius];
                    const PixelHDR& p = src.at(sx, y);
                    acc.r += w * p.r; acc.g += w * p.g; acc.b += w * p.b;
                }
                dst.at(x, y) = acc;
            }
        }
        return dst;
    }

    inline ImageBuffer blur_vertical(const ImageBuffer& src, int radius, double sigma = 0.0) {
        if (sigma <= 0.0) sigma = radius / 3.0;
        auto k = make_gaussian_kernel_1d(radius, sigma);

        ImageBuffer dst;
        dst.width  = src.width;
        dst.height = src.height;
        dst.pixels.resize(static_cast<size_t>(src.width) * src.height);

        for (int y = 0; y < src.height; ++y) {
            for (int x = 0; x < src.width; ++x) {
                PixelHDR acc{0.0, 0.0, 0.0};
                for (int dy = -radius; dy <= radius; ++dy) {
                    int sy = std::clamp(y + dy, 0, src.height - 1);
                    double w = k[dy + radius];
                    const PixelHDR& p = src.at(x, sy);
                    acc.r += w * p.r; acc.g += w * p.g; acc.b += w * p.b;
                }
                dst.at(x, y) = acc;
            }
        }
        return dst;
    }

    inline PostPass bloom_pass(double threshold = 1.5, int radius = 7, double strength = 0.3) {
        return [threshold, radius, strength](ImageBuffer& buf) {
            ImageBuffer bright = extract_bright(buf, threshold);

            ImageBuffer blurred = blur_horizontal(bright, radius);
            blurred = blur_vertical(blurred, radius);

            for (int y = 0; y < buf.height; y++)
                for (int x = 0; x < buf.width; x++)
                    buf.at(x, y) += blurred.at(x, y) * strength;
        };
    }

    inline PostPass ssao_pass(
        std::vector<double> depth,
        std::vector<Vec3d> normals,
        std::vector<Vec3d> positions,
        int screen_radius = 20,
        int num_samples = 32,
        double occlusion_radius = 0.5,
        double strength = 1.5)
    {
        return [depth=std::move(depth), normals=std::move(normals), positions=std::move(positions),
                screen_radius, num_samples, occlusion_radius, strength]
               (ImageBuffer& buf) {
            std::mt19937 rng_ssao(std::random_device{}());
            std::uniform_real_distribution<double> dist_angle(0.0, 2.0 * pi);
            std::uniform_real_distribution<double> dist_r(0.0, 1.0);

            const double inf = std::numeric_limits<double>::infinity();
            int w = buf.width, h = buf.height;
            std::vector<double> ao(w * h, 1.0);

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    int idx = y * w + x;
                    if (depth[idx] >= inf) continue;

                    Vec3d N = normals[idx];
                    if (N.length() < 0.1) continue;
                    N = N.normalized();
                    const Vec3d& P = positions[idx];

                    double occlusion = 0.0;
                    for (int s = 0; s < num_samples; ++s) {
                        double angle = dist_angle(rng_ssao);
                        double r = std::sqrt(dist_r(rng_ssao)) * screen_radius;
                        int sx = std::clamp(x + (int)(r * std::cos(angle)), 0, w - 1);
                        int sy = std::clamp(y + (int)(r * std::sin(angle)), 0, h - 1);
                        int sidx = sy * w + sx;

                        if (depth[sidx] >= inf) continue;

                        Vec3d dir = positions[sidx] - P;
                        double dist3d = dir.length();
                        if (dist3d < 1e-6 || dist3d > occlusion_radius) continue;

                        dir = dir / dist3d;
                        double contrib = std::max(0.0, dot(N, dir));
                        occlusion += contrib * (1.0 - dist3d / occlusion_radius);
                    }
                    ao[idx] = std::max(0.0, 1.0 - strength * occlusion / num_samples);
                }
            }

            auto bilateral_blur = [&](const std::vector<double>& src, int radius) {
                std::vector<double> dst(w * h, 1.0);
                for (int y = 0; y < h; ++y) {
                    for (int x = 0; x < w; ++x) {
                        int idx = y * w + x;
                        if (depth[idx] >= inf) { dst[idx] = 1.0; continue; }
                        double d_center = depth[idx];
                        double sum = 0.0, wsum = 0.0;
                        for (int dy = -radius; dy <= radius; ++dy) {
                            for (int dx = -radius; dx <= radius; ++dx) {
                                int nx = std::clamp(x + dx, 0, w - 1);
                                int ny = std::clamp(y + dy, 0, h - 1);
                                int nidx = ny * w + nx;
                                if (depth[nidx] >= inf) continue;
                                if (std::abs(depth[nidx] - d_center) > 0.5) continue;
                                double ws = 1.0 / (1.0 + dx*dx + dy*dy);
                                sum  += src[nidx] * ws;
                                wsum += ws;
                            }
                        }
                        dst[idx] = wsum > 0.0 ? sum / wsum : src[idx];
                    }
                }
                return dst;
            };

            auto ao_blurred = bilateral_blur(bilateral_blur(ao, 3), 3);

            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x) {
                    double f = ao_blurred[y * w + x];
                    buf.at(x, y).r *= f;
                    buf.at(x, y).g *= f;
                    buf.at(x, y).b *= f;
                }
        };
    }
}
