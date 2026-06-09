#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <map>
#include <cmath>
#include "scene/scene_loader.h"
#include "camera/camera.h"
#include "materials/materials.h"

using namespace rt;

std::mutex cerr_mutex;

struct Pixel { int r, g, b; };

static double clamp(double v, double lo, double hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

static Vec3d gamma(Vec3d v, double g) {
    v = v / 255;
    return 255 * Vec3d(std::pow(v.x, 1.0/g), std::pow(v.y, 1.0/g), std::pow(v.z, 1.0/g));
}

static Vec3d ray_color(const Ray& ray, const Scene& scene, int depth) {
    Vec3d norm = ray.direction.normalized();
    double y   = (norm.y + 1) / 2;
    Vec3d sky  = (1-y) * Vec3d(255,255,255) + y * Vec3d(128,178,255);

    if (depth == 0) return sky;

    if (auto rec = scene.hit(ray, 0.001, 1000)) {
        Vec3d emitted = rec->emission;
        if (auto scatter = rec->material(ray, *rec))
            return emitted + mul(scatter->attenuation / 255.0,
                                 ray_color(scatter->scattered, scene, depth - 1));
        return emitted;
    }
    return sky;
}

static void render_rows(Camera camera, int row_start, int row_end, int width, int height,
                        const Scene& scene, std::vector<Pixel>& pixels,
                        std::atomic<int>& lines_done, int samples)
{
    for (int i = row_start; i < row_end; i++) {
        for (int j = 0; j < width; j++) {
            Vec3d color(0, 0, 0);
            for (int s = 0; s < samples; s++) {
                double u = ((double)j + rng_dist01(rng)) / (width  - 1);
                double v = ((double)i + rng_dist01(rng)) / (height - 1);
                color += ray_color(camera.get_ray(u, v), scene, 8);
            }
            color = gamma(color / samples, 2.2);
            color = Vec3d(clamp(color.x, 0, 255), clamp(color.y, 0, 255), clamp(color.z, 0, 255));
            pixels[i * width + j] = Pixel{(int)color.x, (int)color.y, (int)color.z};
        }
        ++lines_done;
        {
            std::lock_guard<std::mutex> lock(cerr_mutex);
            std::cerr << "\rLignes traitées : " << lines_done << "/" << height << std::flush;
        }
    }
}

int main(int argc, char* argv[]) {
    double      multiplier = argc > 1 ? std::stod(argv[1]) : 1.0;
    int         samples    = argc > 2 ? std::stoi(argv[2]) : 64;
    std::string scene_path = argc > 3 ? argv[3] : "scenes/scene_dof.txt";
    const int width        = (int)(400 * multiplier);
    const int height       = (int)(225 * multiplier);
    const double aspect_ratio = (double)width / height;

    Camera camera;
    std::map<std::string, Material> materials = build_materials();
    Scene scene = load_scene(scene_path, materials, camera, aspect_ratio);
    scene.build_bvh();

    std::vector<Pixel> pixels(width * height);
    std::atomic<int>   lines_done(0);

    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    int rows_per_thread = height / num_threads;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; i++) {
        int end = (i == num_threads - 1) ? height : (i + 1) * rows_per_thread;
        threads.emplace_back(render_rows, camera, i * rows_per_thread, end,
                             width, height, std::cref(scene),
                             std::ref(pixels), std::ref(lines_done), samples);
    }
    for (auto& t : threads) t.join();

    std::cout << "P3\n" << width << " " << height << "\n255\n";
    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            const Pixel& p = pixels[i * width + j];
            std::cout << p.r << " " << p.g << " " << p.b << " ";
        }
        std::cout << "\n";
    }
    std::cerr << std::endl;
    return 0;
}
