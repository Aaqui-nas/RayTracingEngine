#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <map>
#include <random>
#include "vec3.h"
#include "ray.h"
#include "scene.h"
#include "scene_loader.h"

const Vec3d origin(0, 0, 0);
const Vec3d horizontal(4.0, 0, 0);
const Vec3d vertical(0, 2.25, 0);
const Vec3d lower_left_corner = origin - horizontal/2 - vertical/2 - Vec3d(0,0,1);
std::mutex cerr_mutex;

struct Pixel { int r, g, b; };

double clamp(double v, double min, double max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

Vec3d gammaVec(Vec3d v, double g) {
    v = v / 255;
    return 255 * Vec3d(std::pow(v.x, 1/g), std::pow(v.y, 1/g), std::pow(v.z, 1/g));
}

thread_local std::mt19937 rng(std::random_device{}());
thread_local std::uniform_real_distribution<double> rng_dist(-1.0, 1.0);
thread_local std::uniform_real_distribution<double> rng_dist01(0.0, 1.0);

Vec3d random_in_unit_sphere() {
    Vec3d p;
    do { p = Vec3d(rng_dist(rng), rng_dist(rng), rng_dist(rng)); } while (dot(p,p) >= 1.0);
    return p;
}

Vec3d random_unit_vector() {
    Vec3d v = random_in_unit_sphere();
    return v / std::sqrt(dot(v, v));
}

Vec3d reflect(Vec3d v, Vec3d n) {
    return v - 2 * dot(v, n) * n;
}

Vec3d refract(Vec3d uv, Vec3d n, double ri) {
    double cos_theta = std::min(dot(-uv, n), 1.0);
    Vec3d perp = ri * (uv + cos_theta * n);
    Vec3d parallel = -std::sqrt(std::abs(1.0 - dot(perp, perp))) * n;
    return perp + parallel;
}

double schlick(double cosine, double ri) {
    double r0 = (1 - ri) / (1 + ri);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow(1 - cosine, 5);
}

Vec3d mul(Vec3d a, Vec3d b) { return Vec3d(a.x*b.x, a.y*b.y, a.z*b.z); }

Vec3d ray_color(const Ray& ray, const Scene& scene, int depth) {
    if (depth == 0) return Vec3d(0, 0, 0);
    if (auto hit = scene.hit(ray, 0.001, 1000)) {
        auto [rec, idx] = *hit;
        Vec3d emitted = scene.objects[idx]->emission;
        if (auto scatter = scene.objects[idx]->material(ray, rec))
            return emitted + mul(scatter->attenuation / 255.0, ray_color(scatter->scattered, scene, depth - 1));
        return emitted;
    }
    Vec3d norm = ray.direction.normalized();
    double y = (norm.y + 1) / 2;
    return (1-y)*Vec3d(255,255,255) + y*Vec3d(128,178,255);
}

// TODO: adapte cette fonction pour qu'elle rende les lignes [row_start, row_end[
// et écrive le résultat dans pixels[row_start * width .. row_end * width]
void render_rows(int row_start, int row_end, int width, int height,
                 const Scene& scene, std::vector<Pixel>& pixels,
                 std::atomic<int>& lines_done, int samples) {
    // TODO: boucle sur i dans [row_start, row_end[
    //       boucle sur j dans [0, width[
    //       calcule le rayon, la couleur, stocke dans pixels[i*width + j]
    //       lines_done++ après chaque ligne
    for (int i = row_start; i < row_end; i++) {
        for (int j = 0; j < width; j++) {
            Vec3d color(0, 0, 0);
            for (int s = 0; s < samples; s++) {
                double u = ((double)j + rng_dist01(rng)) / (width - 1);
                double v = ((double)i + rng_dist01(rng)) / (height - 1);
                Ray ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
                color += ray_color(ray, scene, 8);
            }
            color = gammaVec(color / samples, 2.2);
            color = Vec3d(clamp(color.x, 0, 255), clamp(color.y, 0, 255), clamp(color.z, 0, 255));
            pixels[i*width+j]=Pixel{(int)color.x,(int)color.y,(int)color.z};
        }
        lines_done++;
        {
            std::lock_guard<std::mutex> lock(cerr_mutex);
            std::cerr << "\rLignes traitées : " << lines_done << "/" << height << std::flush;
        }
    }
}


int main(int argc, char* argv[]) {
    double multiplier = argc > 1 ? std::stod(argv[1]) : 1.0;
    int    samples    = argc > 2 ? std::stoi(argv[2]) : 64;
    const int width  = (int)(400 * multiplier);
    const int height = (int)(225 * multiplier);

    std::map<std::string, Material> materials;

    auto lambertian = [](Vec3d color) -> Material {
        return [color](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
            Vec3d dir = rec.normal + random_unit_vector();
            if (dot(dir, dir) < 1e-8) dir = rec.normal;
            return Scatter{color, Ray(rec.point, dir)};
        };
    };
    auto metal = [](Vec3d color, double fuzz) -> Material {
        return [color, fuzz](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
            Vec3d reflected = reflect(r.direction.normalized(), rec.normal);
            reflected = reflected + fuzz * random_in_unit_sphere();
            if (dot(reflected, rec.normal) <= 0) return std::nullopt;
            return Scatter{color, Ray(rec.point, reflected)};
        };
    };
    auto glass = [](double ior) -> Material {
        return [ior](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
            double ri = rec.front_face ? 1.0 / ior : ior;
            Vec3d unit_dir = r.direction.normalized();
            double cos_theta = std::min(dot(-unit_dir, rec.normal), 1.0);
            double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
            Vec3d dir = (ri * sin_theta > 1.0 || rng_dist01(rng) < schlick(cos_theta, ri))
                ? reflect(unit_dir, rec.normal)
                : refract(unit_dir, rec.normal, ri);
            return Scatter{Vec3d(255, 255, 255), Ray(rec.point, dir)};
        };
    };

    auto tinted_glass = [glass](Vec3d tint, double ior) -> Material {
        auto base = glass(ior);
        return [base, tint](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
            auto result = base(r, rec);
            if (result) result->attenuation = tint;
            return result;
        };
    };

    materials["red"]      = lambertian(Vec3d(255, 50, 50));
    materials["green"]    = lambertian(Vec3d(50, 200, 80));
    materials["blue"]     = lambertian(Vec3d(50, 100, 255));
    materials["white"]    = lambertian(Vec3d(240, 240, 240));
    materials["normal"]   = [](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
        Vec3d n = rec.normal;
        Vec3d color = 255 * Vec3d((n.x+1)/2, (n.y+1)/2, (n.z+1)/2);
        return Scatter{color, Ray(rec.point, rec.normal + random_unit_vector())};
    };
    materials["gradient"] = [](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
        double y = (rec.normal.y + 1) / 2;
        Vec3d color = (1-y) * Vec3d(255, 220, 0) + y * Vec3d(255, 255, 255);
        return Scatter{color, Ray(rec.point, rec.normal + random_unit_vector())};
    };
    materials["silver"]      = metal(Vec3d(200, 200, 210), 0.05);
    materials["gold"]        = metal(Vec3d(255, 190, 40), 0.08);
    materials["copper"]      = metal(Vec3d(220, 120, 60), 0.12);
    materials["mirror"]      = metal(Vec3d(255, 255, 255), 0.0);
    materials["fuzzy_metal"] = metal(Vec3d(180, 180, 195), 0.4);
    materials["glass"]       = glass(1.5);
    materials["glass_thick"] = glass(2.4);
    materials["glass_blue"]  = tinted_glass(Vec3d(150, 200, 255), 1.5);
    materials["glass_rose"]  = tinted_glass(Vec3d(255, 165, 180), 1.5);
    materials["__black__"]   = [](const Ray&, const HitRecord&) -> std::optional<Scatter> {
        return std::nullopt;
    };
    materials["checker"]     = [](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
        int ix = (int)std::floor(rec.point.x * 2);
        int iz = (int)std::floor(rec.point.z * 2);
        Vec3d color = ((ix + iz) % 2 == 0) ? Vec3d(240, 240, 240) : Vec3d(30, 30, 30);
        Vec3d dir = rec.normal + random_unit_vector();
        if (dot(dir, dir) < 1e-8) dir = rec.normal;
        return Scatter{color, Ray(rec.point, dir)};
    };
    materials["default"]     = lambertian(Vec3d(200, 200, 200));

    // TODO: charge la scène
    Scene scene = load_scene("scene.txt", materials);

    std::vector<Pixel> pixels(width * height);
    std::atomic<int> lines_done(0);

    // TODO: détecte le nombre de cœurs
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    int num_row = height / num_threads;

    // TODO: lance les threads, chacun sur une bande de lignes
    std::vector<std::thread> threads;
    for (int i =0; i<num_threads; i++){
        int end = (i == num_threads - 1) ? height : (i+1)*num_row;
        threads.push_back(std::thread(render_rows, i*num_row, end, width, height, std::cref(scene), std::ref(pixels), std::ref(lines_done), samples));
    }


    // TODO: join tous les threads
    for (int i =0; i<num_threads; i++){
        threads[i].join();
    }

    // Écriture du fichier
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
