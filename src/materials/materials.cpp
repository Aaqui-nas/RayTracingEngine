#include "materials/materials.h"
#include "materials/texture.h"
#include "core/mat3.h"

namespace rt {

    static Vec3d random_in_unit_sphere() {
        Vec3d p;
        do { p = Vec3d(rng_dist(rng), rng_dist(rng), rng_dist(rng)); } while (dot(p, p) >= 1.0);
        return p;
    }

    static Vec3d random_unit_vector() {
        Vec3d v = random_in_unit_sphere();
        return v / std::sqrt(dot(v, v));
    }

    static Vec3d reflect(Vec3d v, Vec3d n) {
        return v - 2 * dot(v, n) * n;
    }

    static Vec3d refract(Vec3d uv, Vec3d n, double ri) {
        double cos_theta = std::min(dot(-uv, n), 1.0);
        Vec3d perp     = ri * (uv + cos_theta * n);
        Vec3d parallel = -std::sqrt(std::abs(1.0 - dot(perp, perp))) * n;
        return perp + parallel;
    }

    static double schlick(double cosine, double ri) {
        double r0 = (1 - ri) / (1 + ri);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow(1 - cosine, 5);
    }

    static auto lambertian = [](TexturePtr tex) -> Material {
        return [tex](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
            Vec3d color = tex->sample(rec.u, rec.v, rec.point);
            Vec3d dir   = rec.normal + random_unit_vector();
            if (dot(dir, dir) < 1e-8) dir = rec.normal;
            return Scatter{color, Ray(rec.point, dir)};
        };
    };

    static auto metal = [](Vec3d color, double fuzz) -> Material {
        return [color, fuzz](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
            Vec3d reflected = reflect(r.direction.normalized(), rec.normal);
            reflected = reflected + fuzz * random_in_unit_sphere();
            if (dot(reflected, rec.normal) <= 0) return std::nullopt;
            return Scatter{color, Ray(rec.point, reflected)};
        };
    };

    static auto glass = [](double ior) -> Material {
        return [ior](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
            double ri        = rec.front_face ? 1.0 / ior : ior;
            Vec3d  unit_dir  = r.direction.normalized();
            double cos_theta = std::min(dot(-unit_dir, rec.normal), 1.0);
            double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
            Vec3d dir = (ri * sin_theta > 1.0 || rng_dist01(rng) < schlick(cos_theta, ri))
                ? reflect(unit_dir, rec.normal)
                : refract(unit_dir, rec.normal, ri);
            return Scatter{Vec3d(255, 255, 255), Ray(rec.point, dir)};
        };
    };

    static auto tinted_glass = [](Vec3d tint, double ior) -> Material {
        auto base = glass(ior);
        return [base, tint](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
            auto result = base(r, rec);
            if (result) result->attenuation = tint;
            return result;
        };
    };

    Material normal_mapped(Material base, TexturePtr nmap) {
        return [base, nmap](const Ray& r, const HitRecord& rec) -> std::optional<Scatter> {
            Vec3d grad = nmap->sample(rec.u, rec.v, rec.point);

            Vec3d T = rec.tangent;
            Vec3d N = rec.normal;
            Vec3d B = N.cross(T);
            Mat3  tbn{T, B, N};

            double px = std::clamp(-dot(grad, T), -1.0, 1.0);
            double py = std::clamp(-dot(grad, B), -1.0, 1.0);
            Vec3d ts = Vec3d(px, py, 1.0).normalized();

            HitRecord perturbed = rec;
            Vec3d perturbed_n = (tbn * ts).normalized();
            if (dot(r.direction, perturbed_n) >= 0)
                perturbed_n = N;
            perturbed.normal = perturbed_n;
            return base(r, perturbed);
        };
    }

    std::map<std::string, Material> build_materials() {
        std::map<std::string, Material> materials;

        materials["red"]      = lambertian(std::make_shared<SolidColor>(Vec3d(255, 50, 50)));
        materials["green"]    = lambertian(std::make_shared<SolidColor>(Vec3d(50, 200, 80)));
        materials["blue"]     = lambertian(std::make_shared<SolidColor>(Vec3d(50, 100, 255)));
        materials["white"]    = lambertian(std::make_shared<SolidColor>(Vec3d(240, 240, 240)));
        materials["normal"]   = [](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
            Vec3d n     = rec.normal;
            Vec3d color = 255 * Vec3d((n.x+1)/2, (n.y+1)/2, (n.z+1)/2);
            return Scatter{color, Ray(rec.point, rec.normal + random_unit_vector())};
        };
        materials["gradient"] = [](const Ray&, const HitRecord& rec) -> std::optional<Scatter> {
            double y    = (rec.normal.y + 1) / 2;
            Vec3d color = (1-y) * Vec3d(255, 220, 0) + y * Vec3d(255, 255, 255);
            return Scatter{color, Ray(rec.point, rec.normal + random_unit_vector())};
        };
        materials["silver"]      = metal(Vec3d(200, 200, 210), 0.05);
        materials["gold"]        = metal(Vec3d(255, 190, 40),  0.08);
        materials["copper"]      = metal(Vec3d(220, 120, 60),  0.12);
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
            Vec3d dir   = rec.normal + random_unit_vector();
            if (dot(dir, dir) < 1e-8) dir = rec.normal;
            return Scatter{color, Ray(rec.point, dir)};
        };
        materials["default"]     = lambertian(std::make_shared<SolidColor>(Vec3d(200, 200, 200)));

        materials["checker_uv"]  = lambertian(std::make_shared<CheckerTexture>(
            Vec3d(245, 245, 245), Vec3d(20, 20, 20), 10.0));
        materials["checker_orange"] = lambertian(std::make_shared<CheckerTexture>(
            Vec3d(255, 140, 20), Vec3d(40, 20, 10), 8.0));
        materials["checker_blue_uv"] = lambertian(std::make_shared<CheckerTexture>(
            Vec3d(30, 90, 220), Vec3d(220, 240, 255), 8.0));

        materials["perlin_marble"] = lambertian(std::make_shared<PerlinTexture>(
            Vec3d(220, 215, 200), 5.0));
        materials["perlin_lava"]   = lambertian(std::make_shared<PerlinTexture>(
            Vec3d(255, 80, 10), 6.0));
        materials["perlin_moss"]   = lambertian(std::make_shared<PerlinTexture>(
            Vec3d(80, 160, 60), 4.0));

        materials["bump_marble"] = normal_mapped(
            lambertian(std::make_shared<SolidColor>(Vec3d(200, 200, 200))),
            std::make_shared<BumpTexture>(4.0, 1.5)
        );
        materials["bump_rocky"] = normal_mapped(
            lambertian(std::make_shared<SolidColor>(Vec3d(140, 110, 80))),
            std::make_shared<BumpTexture>(8.0, 3.0)
        );
        materials["bump_gold"] = normal_mapped(
            metal(Vec3d(255, 190, 40), 0.05),
            std::make_shared<BumpTexture>(1.0, 1)
        );
        return materials;
    }

}
