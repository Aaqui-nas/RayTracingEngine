#pragma once
#include <immintrin.h>
#include "geometry/sphere.h"

namespace rt {
    struct Vec3x4 {
        alignas(32) double x[4];
        alignas(32) double y[4];
        alignas(32) double z[4];
    };

    struct Ray4
    {
        Vec3x4 origin;
        Vec3x4 direction;
    };


    void dot4(const Vec3x4& a, const Vec3x4& b, double results[4]) {
        __m256d ax = _mm256_load_pd(a.x);
        __m256d bx = _mm256_load_pd(b.x);
        __m256d ay = _mm256_load_pd(a.y);
        __m256d by = _mm256_load_pd(b.y);
        __m256d az = _mm256_load_pd(a.z);
        __m256d bz = _mm256_load_pd(b.z);
        __m256d xx = _mm256_mul_pd(ax, bx);
        __m256d yy= _mm256_mul_pd(ay, by);
        __m256d zz = _mm256_mul_pd(az, bz);
        _mm256_store_pd(results, _mm256_add_pd(xx, _mm256_add_pd(yy, zz)));

    };

    void length_sq4(const Vec3x4& v, double results[4]) {
        dot4(v, v, results);
    }

    Vec3x4 normalize4(const Vec3x4& v) {
        __m256d ax = _mm256_load_pd(v.x);
        __m256d ay = _mm256_load_pd(v.y);
        __m256d az = _mm256_load_pd(v.z);

        __m256d len_sq = _mm256_add_pd(
            _mm256_mul_pd(ax, ax),
            _mm256_add_pd(_mm256_mul_pd(ay, ay), _mm256_mul_pd(az, az)));

        __m256d len = _mm256_sqrt_pd(len_sq);

        Vec3x4 out;
        _mm256_store_pd(out.x, _mm256_div_pd(ax, len));
        _mm256_store_pd(out.y, _mm256_div_pd(ay, len));
        _mm256_store_pd(out.z, _mm256_div_pd(az, len));
        return out;
    }

    // results[i] = t si le rayon i touche la sphère, -1.0 sinon
    void hit_sphere4(const Ray4& ray, const Sphere& sphere, double results[4]) {
        __m256d cx = _mm256_set1_pd(sphere.center.x);
        __m256d cy = _mm256_set1_pd(sphere.center.y);
        __m256d cz = _mm256_set1_pd(sphere.center.z);

        __m256d ocx = _mm256_sub_pd(_mm256_load_pd(ray.origin.x), cx);
        __m256d ocy = _mm256_sub_pd(_mm256_load_pd(ray.origin.y), cy);
        __m256d ocz = _mm256_sub_pd(_mm256_load_pd(ray.origin.z), cz);

        __m256d dx = _mm256_load_pd(ray.direction.x);
        __m256d dy = _mm256_load_pd(ray.direction.y);
        __m256d dz = _mm256_load_pd(ray.direction.z);

        __m256d a = _mm256_add_pd(_mm256_mul_pd(dx, dx),
                    _mm256_add_pd(_mm256_mul_pd(dy, dy), _mm256_mul_pd(dz, dz)));

        __m256d half_b = _mm256_add_pd(_mm256_mul_pd(ocx, dx),
                         _mm256_add_pd(_mm256_mul_pd(ocy, dy), _mm256_mul_pd(ocz, dz)));

        __m256d r2 = _mm256_set1_pd(sphere.radius * sphere.radius);
        __m256d c = _mm256_sub_pd(
            _mm256_add_pd(_mm256_mul_pd(ocx, ocx),
            _mm256_add_pd(_mm256_mul_pd(ocy, ocy), _mm256_mul_pd(ocz, ocz))),
            r2);

        __m256d disc = _mm256_sub_pd(_mm256_mul_pd(half_b, half_b),
                                     _mm256_mul_pd(a, c));

        __m256d zero = _mm256_setzero_pd();
        __m256d hit_mask = _mm256_cmp_pd(disc, zero, _CMP_GE_OQ);

        __m256d sqrt_disc = _mm256_sqrt_pd(_mm256_max_pd(disc, zero)); // clamp évite NaN
        __m256d neg_half_b = _mm256_sub_pd(zero, half_b);
        __m256d t = _mm256_div_pd(_mm256_sub_pd(neg_half_b, sqrt_disc), a);

        __m256d miss = _mm256_set1_pd(-1.0);
        _mm256_store_pd(results, _mm256_blendv_pd(miss, t, hit_mask));
    }
}
