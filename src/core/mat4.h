#pragma once
#include "core/vec3.h"

namespace rt{
    struct Mat4 {
        double m[4][4];

        static Mat4 identity() {
            return {{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
        };

        static Mat4 translate(double dx, double dy, double dz) {
            return {{{1,0,0,dx},{0,1,0,dy},{0,0,1,dz},{0,0,0,1}}};
        };

        static Mat4 scale(double sx, double sy, double sz) {
            return {{{sx,0,0,0},{0,sy,0,0},{0,0,sz,0},{0,0,0,1}}};
        };

        static Mat4 rotate_x(double angle_rad) {
            return {{{1,0,0,0},{0,cos(angle_rad),-sin(angle_rad),0},{0,sin(angle_rad),cos(angle_rad),0},{0,0,0,1}}};
        };

        static Mat4 rotate_y(double angle_rad) {
            return {{{cos(angle_rad),0,sin(angle_rad),0},{0,1,0,0},{-sin(angle_rad),0,cos(angle_rad),0},{0,0,0,1}}};
        };

        static Mat4 rotate_z(double angle_rad) {
            return {{{cos(angle_rad),-sin(angle_rad),0,0},{sin(angle_rad),cos(angle_rad),0,0},{0,0,1,0},{0,0,0,1}}};
        };

        Mat4 operator*(const Mat4& other) const {
            Mat4 result{};

            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result.m[i][j] = 0.0;
                    for (int k = 0; k < 4; ++k) {
                        result.m[i][j] += m[i][k] * other.m[k][j];
                    }
                }
            }

            return result;
        }
        Mat4 inverse() const {
            Mat4 inv{};
            double a = m[0][0], b = m[0][1], c = m[0][2];
            double d = m[1][0], e = m[1][1], f = m[1][2];
            double g = m[2][0], h = m[2][1], i = m[2][2];

            double det =
                a * (e * i - f * h)
            - b * (d * i - f * g)
            + c * (d * h - e * g);

            if (std::abs(det) < 1e-12)
                throw std::runtime_error("Matrix not invertible");

            double invDet = 1.0 / det;
            inv.m[0][0] =  (e*i - f*h) * invDet;
            inv.m[0][1] = -(b*i - c*h) * invDet;
            inv.m[0][2] =  (b*f - c*e) * invDet;

            inv.m[1][0] = -(d*i - f*g) * invDet;
            inv.m[1][1] =  (a*i - c*g) * invDet;
            inv.m[1][2] = -(a*f - c*d) * invDet;

            inv.m[2][0] =  (d*h - e*g) * invDet;
            inv.m[2][1] = -(a*h - b*g) * invDet;
            inv.m[2][2] =  (a*e - b*d) * invDet;

            double tx = m[0][3];
            double ty = m[1][3];
            double tz = m[2][3];

            inv.m[0][3] = -(inv.m[0][0]*tx + inv.m[0][1]*ty + inv.m[0][2]*tz);
            inv.m[1][3] = -(inv.m[1][0]*tx + inv.m[1][1]*ty + inv.m[1][2]*tz);
            inv.m[2][3] = -(inv.m[2][0]*tx + inv.m[2][1]*ty + inv.m[2][2]*tz);

            inv.m[3][0] = 0;
            inv.m[3][1] = 0;
            inv.m[3][2] = 0;
            inv.m[3][3] = 1;

            return inv;
        }
        Mat4 transpose() const {
            Mat4 result{};

            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result.m[i][j] = m[j][i];
                }
            }

            return result;
        }

        Vec3d transform_point(const Vec3d& p) const {
            return Vec3d(m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z + m[0][3]*1,
                        m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z + m[1][3]*1,
                        m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z + m[2][3]*1);
        }
        Vec3d transform_dir(const Vec3d& d) const {
            return Vec3d(m[0][0]*d.x + m[0][1]*d.y + m[0][2]*d.z,
                        m[1][0]*d.x + m[1][1]*d.y + m[1][2]*d.z,
                        m[2][0]*d.x + m[2][1]*d.y + m[2][2]*d.z);
        }
    };
}
