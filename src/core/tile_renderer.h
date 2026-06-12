#pragma once

#include <vector>
#include <future>
#include <algorithm>
#include <limits>
#include "core/renderer.h"
#include "core/post_process.h"
#include "core/work_stealing_pool.h"
#include "scene/scene.h"
#include "camera/camera.h"

namespace rt {

struct Pixel { int r, g, b; };

struct PixelBuffer {
    int width, height;
    std::vector<Pixel> pixels;
    PixelBuffer(int w, int h) : width(w), height(h), pixels(w * h) {}
};

struct Tile {
    int x0, y0;
    int x1, y1;
};

struct RenderOutput {
    ImageBuffer color;
    std::vector<double> depth;
    std::vector<Vec3d> normals;
    std::vector<Vec3d> positions;
};

inline std::vector<Tile> make_tiles(int width, int height, int tile_w, int tile_h) {
    std::vector<Tile> tiles;
    for (int y = 0; y < height; y += tile_h)
        for (int x = 0; x < width; x += tile_w)
            tiles.push_back({x, y,
                             std::min(x + tile_w, width),
                             std::min(y + tile_h, height)});
    return tiles;
}

inline void render_tile(const Scene& scene, const Camera& camera,
                        PixelBuffer& buffer, const Tile& tile, int samples) {
    for (int i = tile.y0; i < tile.y1; i++) {
        for (int j = tile.x0; j < tile.x1; j++) {
            Vec3d color(0, 0, 0);
            for (int s = 0; s < samples; s++) {
                double u = ((double)j + rng_dist01(rng)) / (buffer.width  - 1);
                double v = ((double)i + rng_dist01(rng)) / (buffer.height - 1);
                color += ray_color(camera.get_ray(u, v), scene, 8);
            }
            color = gamma(color / samples, 2.2);
            color = Vec3d(clamp(color.x, 0, 255), clamp(color.y, 0, 255), clamp(color.z, 0, 255));
            buffer.pixels[i * buffer.width + j] = Pixel{(int)color.x, (int)color.y, (int)color.z};
        }
    }
}

inline void render_tile_hdr(const Scene& scene, const Camera& camera,
                             RenderOutput& out, const Tile& tile, int samples) {
    int w = out.color.width, h = out.color.height;
    for (int y = tile.y0; y < tile.y1; ++y) {
        for (int x = tile.x0; x < tile.x1; ++x) {
            int idx = y * w + x;

            double u0 = (x + 0.5) / (w - 1);
            double v0 = (y + 0.5) / (h - 1);
            auto gb = sample_gbuffer(camera.get_ray(u0, v0), scene);
            out.depth[idx]     = gb.t;
            out.normals[idx]   = gb.normal;
            out.positions[idx] = gb.point;

            Vec3d color(0, 0, 0);
            for (int s = 0; s < samples; ++s) {
                double u = (x + rng_dist01(rng)) / (w - 1);
                double v = (y + rng_dist01(rng)) / (h - 1);
                color += ray_color(camera.get_ray(u, v), scene, 8);
            }
            color /= samples;
            out.color.at(x, y) = PixelHDR{color.x / 255.0, color.y / 255.0, color.z / 255.0};
        }
    }
}

inline void render_tiles(const Scene& scene, const Camera& camera,
                         PixelBuffer& buffer, int samples, int tile_size, bool spiral) {
    WorkStealingPool pool(std::thread::hardware_concurrency());
    auto tiles = make_tiles(buffer.width, buffer.height, tile_size, tile_size);
    if (spiral) {
        auto cx = buffer.width / 2.0, cy = buffer.height / 2.0;
        std::sort(tiles.begin(), tiles.end(), [cx, cy](const Tile& a, const Tile& b) {
            auto dist = [cx, cy](const Tile& t) {
                double dx = (t.x0 + t.x1) / 2.0 - cx;
                double dy = (t.y0 + t.y1) / 2.0 - cy;
                return dx*dx + dy*dy;
            };
            return dist(a) < dist(b);
        });
    }
    std::atomic<int> tiles_done{0};
    int total_tiles = tiles.size();
    std::vector<std::future<void>> futures;
    for (Tile tile : tiles) {
        futures.push_back(pool.submit([&scene, &camera, &buffer, tile, samples, &tiles_done, total_tiles]() {
            render_tile(scene, camera, buffer, tile, samples);
            int done = ++tiles_done;
            if (done % 10 == 0 || done == total_tiles) {
                std::fprintf(stderr, "\rRendu : %d/%d tuiles (%.1f%%)",
                    done, total_tiles, 100.0 * done / total_tiles);
                std::fflush(stderr);
            }
        }));
    }
    for (auto& f : futures) f.get();
}

inline RenderOutput render_tiles_hdr(const Scene& scene, const Camera& camera,
                                     int width, int height, int samples,
                                     int tile_size, bool spiral) {
    RenderOutput out;
    out.color   = ImageBuffer{width, height, std::vector<PixelHDR>((size_t)width * height)};
    out.depth.assign((size_t)width * height, std::numeric_limits<double>::infinity());
    out.normals.resize((size_t)width * height);
    out.positions.resize((size_t)width * height);

    WorkStealingPool pool(std::thread::hardware_concurrency());
    auto tiles = make_tiles(width, height, tile_size, tile_size);

    if (spiral) {
        auto cx = width / 2.0, cy = height / 2.0;
        std::sort(tiles.begin(), tiles.end(), [cx, cy](const Tile& a, const Tile& b) {
            auto dist = [cx, cy](const Tile& t) {
                double dx = (t.x0 + t.x1) / 2.0 - cx;
                double dy = (t.y0 + t.y1) / 2.0 - cy;
                return dx*dx + dy*dy;
            };
            return dist(a) < dist(b);
        });
    }

    std::atomic<int> tiles_done{0};
    int total_tiles = (int)tiles.size();
    std::vector<std::future<void>> futures;

    for (Tile tile : tiles) {
        futures.push_back(pool.submit([&scene, &camera, &out, tile, samples, &tiles_done, total_tiles]() {
            render_tile_hdr(scene, camera, out, tile, samples);
            int done = ++tiles_done;
            if (done % 10 == 0 || done == total_tiles) {
                std::fprintf(stderr, "\rRendu : %d/%d tuiles (%.1f%%)",
                    done, total_tiles, 100.0 * done / total_tiles);
                std::fflush(stderr);
            }
        }));
    }
    for (auto& f : futures) f.get();
    return out;
}

}
