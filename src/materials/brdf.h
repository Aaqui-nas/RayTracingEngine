#pragma once
#include "core/vec3.h"
#include "geometry/shape.h"
#include "materials/materials.h"
#include <cmath>
#include <algorithm>

namespace rt {
    namespace physics {
        inline double D_GGX(double NdotH, double roughness) {
             double alpha = roughness*roughness;
             double den = pi*pow((NdotH*NdotH)*(alpha*alpha-1)+1,2);
             return alpha*alpha/den;
        }

        inline double G_Smith(double NdotL, double NdotV, double roughness) {
            double alpha = roughness*roughness;
            double k = alpha * alpha / 2;
            auto G1 = [k](double NdotX) -> double {
                return NdotX / (NdotX*(1-k) + k);
            };
            return G1(NdotL) * G1(NdotV);
        }

        inline double G2_SmithCorrelated(double NdotL, double NdotV, double roughness) {
            double alpha = roughness*roughness;
            auto lambda = [alpha](double x) -> double {
                return (-1 + sqrt(1 + alpha*alpha*(1-x*x)/(x*x)))/2;
            };
            return (1 / (1 + lambda(NdotL) + lambda(NdotV)));
        }

        inline Vec3d F_Schlick(double VdotH, const Vec3d& F0) {
            Vec3d F = F0;
            double pw = pow(1 - VdotH, 5);
            F.x += (1 - F0.x) * pw;
            F.y += (1 - F0.y) * pw;
            F.z += (1 - F0.z) * pw;
            return F;
        }

        inline double F0_from_ior(double ior) {
            double r = (ior - 1) / (ior + 1);
            return r * r;
        }

        inline Vec3d brdf_cook_torrance(
            const Vec3d& albedo_linear,
            double roughness,
            double metalness,
            double ior,
            const Vec3d& N,
            const Vec3d& V,
            const Vec3d& L
        ) {
            Vec3d h = (L + V).normalized();
            double NdotH = std::max(0.0, dot(N, h));
            double NdotL = std::max(0.0, dot(N, L));
            double NdotV = std::max(0.0, dot(N, V));
            double VdotH = std::max(0.0, dot(V, h));

            double F0_ = F0_from_ior(ior);
            Vec3d F0 =  Vec3d(F0_, F0_, F0_) * (1-metalness) + albedo_linear * metalness;

            double D = D_GGX(NdotH, roughness);
            Vec3d F = F_Schlick(VdotH, F0);
            double G = G2_SmithCorrelated(NdotL, NdotV, roughness);

            Vec3d f_specular = D * F * G / std::max(4 * NdotL * NdotV, 1e-7);

            double k_d = (1 - metalness) * (1 - (F.x+F.y+F.z)/3);
            Vec3d f_diffuse = k_d * (albedo_linear / pi);

            return f_diffuse + f_specular;
        }
    }

    inline Material cook_torrance(Vec3d albedo, double roughness, double metalness, double ior = 1.5) {
        return [albedo, roughness, metalness, ior](const Ray& r, const HitRecord& rec)
               -> std::optional<Scatter> {
            double alpha = roughness * roughness;
            Vec3d z = rec.normal;
            Vec3d up = std::abs(z.y) < 0.9 ? Vec3d(0,1,0) : Vec3d(1,0,0);
            Vec3d x = z.cross(up).normalized();
            Vec3d y = z.cross(x);
            double xi1 = rng_dist01(rng);
            double xi2 = rng_dist01(rng);
            double cos_theta = sqrt((1 - xi1) / (1 + (alpha*alpha - 1) * xi1));
            double sin_theta = sqrt(1 - pow(cos_theta, 2));
            double phi = 2*pi*xi2;

            Vec3d h_world = sin_theta*cos(phi)*x + sin_theta*sin(phi)*y + cos_theta *z;

            Vec3d V = -r.direction.normalized();
            Vec3d L = reflect(-V, h_world).normalized();
            if (dot(L, rec.normal) <= 0) return std::nullopt;

            double VdotH = dot(V, h_world);
            double NdotH = dot(rec.normal, h_world);
            double NdotL = dot(rec.normal, L);

            double pdf_GGX = physics::D_GGX(NdotH, roughness) * NdotH / (4 * VdotH + 1e-7);
            Vec3d throughput = physics::brdf_cook_torrance(albedo/255.0, roughness, metalness, ior, rec.normal, V, L) * NdotL * 255 / pdf_GGX;
            throughput.x = std::clamp(throughput.x, 0.0, 255.0);
            throughput.y = std::clamp(throughput.y, 0.0, 255.0);
            throughput.z = std::clamp(throughput.z, 0.0, 255.0);

            return Scatter{throughput, Ray(rec.point, L)};
        };
    }
}
