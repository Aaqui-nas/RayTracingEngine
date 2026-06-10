#pragma once
#include "core/vec3.h"
#include <optional>
#include <functional>
#include <vector>
#include <cmath>

namespace rt {
    struct LightSample {
        Vec3d  direction;
        double distance;
        Vec3d  radiance;
        double pdf;
    };

    struct Light {
        std::function<std::optional<LightSample>(const Vec3d& from, double xi1, double xi2)> sample;
        std::function<double(const Vec3d& from, const Vec3d& direction)> pdf;
    };

    inline Light make_sphere_light(Vec3d center, double radius, Vec3d emission) {
        return Light {
            .sample = [center, radius, emission](const Vec3d& from, double xi1, double xi2)
                        -> std::optional<LightSample> {
                            double d = (center - from).length();
                            if (d <= radius) return std::nullopt;
                            double cos_theta_max = sqrt(1 - pow((radius/d),2));
                            double cos_theta = 1 + xi1 * (cos_theta_max - 1);
                            double sin_theta = sqrt(1 - pow(cos_theta, 2));
                            Vec3d z = (center - from).normalized();
                            Vec3d up = std::abs(z.y)<0.9 ? Vec3d(0,1,0) : Vec3d(1,0,0);
                            Vec3d x = z.cross(up).normalized();
                            Vec3d y = z.cross(x);
                            Vec3d world_dir = sin_theta*cos(2*pi*xi2)*x + sin_theta*sin(2*pi*xi2)*y + cos_theta *z;
                            Vec3d oc = from - center;
                            double a = dot(world_dir, world_dir);
                            double b = 2 * dot(world_dir, oc);
                            double c = dot(oc, oc) - radius * radius;
                            double discriminant = b * b - 4 * a * c;
                            if (discriminant < 0) return std::nullopt;
                            double t = (-b - std::sqrt(discriminant)) / (2 * a);
                            return LightSample{world_dir, t, emission, 1.0 / (2*pi*(1-cos_theta_max))};
                         },
            .pdf    = [center, radius](const Vec3d& from, const Vec3d& direction)
                        -> double {
                            double d = (center - from).length();
                            if (d <= radius) return 0;
                            Vec3d z = (center - from).normalized();
                            double cos_theta_max = sqrt(1 - pow(radius/d,2));
                            double cos_theta = dot(direction, z);
                            if (cos_theta < cos_theta_max) return 0;
                            return 1.0 / (2*pi*(1-cos_theta_max));
                        }
        };
    };

    struct LightList {
        std::vector<Light> lights;

        void add(Light l)  { lights.push_back(std::move(l)); }
        bool empty() const { return lights.empty(); }
        int  size()  const { return (int)lights.size(); }

        std::optional<LightSample> sample(const Vec3d& from, double xi0, double xi1, double xi2) const {
            if (lights.empty()) return std::nullopt;
            int N = lights.size();
            int idx = std::min((int)(xi0 * N), N - 1);
            auto ls = lights[idx].sample(from, xi1, xi2);
            if (!ls) return std::nullopt;
            ls->pdf /= N;
            return *ls;
        }

        double pdf(const Vec3d& from, const Vec3d& direction) const {
            if (lights.empty()) return 0.0;
            double acc = 0;
            for (const Light& l : lights) {
                acc += l.pdf(from, direction);
            }
            return acc / lights.size();
        }
    };

    inline double mis_weight(double pdf_a, double pdf_b) {
      if (pdf_a == 0 &&
         pdf_b == 0) return 0;
      return pdf_a*pdf_a / (pdf_a*pdf_a + pdf_b*pdf_b);
    }
}
