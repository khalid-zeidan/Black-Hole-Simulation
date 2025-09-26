#pragma once
#include "allIncludes.h"
#include "Ray.h"

#pragma region old code
//void getDerivatives(const Ray& ray, double R_S, double& d2r, double& d2theta, double& d2phi) {
//
//    double r = ray.r, theta = ray.theta;
//    double dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;
//    double rs = R_S;
//
//    double f = 1.0 - rs / r;
//
//    double dt_dL = ray.E / f;
//
//    if (f == 0.0) return;
//
//    d2r = -(rs / (2.0 * r * r)) * f * dt_dL * dt_dL
//        + (rs / (2.0 * r * r * f)) * dr * dr
//        + r * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi);
//
//    d2theta = -2.0 * dr * dtheta / r
//        + sin(theta) * cos(theta) * dphi * dphi;
//
//    double sin_theta = sin(theta);
//    if (sin_theta == 0.0) sin_theta = 1e-10; // Avoid pole singularity
//
//    d2phi = -2.0 * dr * dphi / r
//        - 2.0 * cos(theta) / sin_theta * dtheta * dphi;
//}
//
//
//// Full 4th-Order Runge-Kutta Step for 6 variables (r, theta, phi, dr, dtheta, dphi)
//void RK4Step(Ray& ray, double R_S, double d_lambda) {
//    if (ray.r <= R_S) return;
//
//    // --- K1 ---
//    double k1_dr = ray.dr;
//    double k1_dtheta = ray.dtheta;
//    double k1_dphi = ray.dphi;
//    double k1_d2r, k1_d2theta, k1_d2phi;
//    getDerivatives(ray, R_S, k1_d2r, k1_d2theta, k1_d2phi);
//
//    // --- K2 ---
//    Ray k2_ray = ray;
//
//    k2_ray.r = ray.r + 0.5 * d_lambda * k1_dr;
//    k2_ray.theta = ray.theta + 0.5 * d_lambda * k1_dtheta;
//    k2_ray.phi = ray.phi + 0.5 * d_lambda * k1_dphi;
//
//    k2_ray.dr = ray.dr + 0.5 * d_lambda * k1_d2r;
//    k2_ray.dtheta = ray.dtheta + 0.5 * d_lambda * k1_d2theta;
//    k2_ray.dphi = ray.dphi + 0.5 * d_lambda * k1_d2phi;
//
//    double k2_d2r, k2_d2theta, k2_d2phi;
//    getDerivatives(k2_ray, R_S, k2_d2r, k2_d2theta, k2_d2phi);
//
//    // --- K3 ---
//    Ray k3_ray = ray;
//
//    k3_ray.r = ray.r + 0.5 * d_lambda * k2_ray.dr;
//    k3_ray.theta = ray.theta + 0.5 * d_lambda * k2_ray.dtheta;
//    k3_ray.phi = ray.phi + 0.5 * d_lambda * k2_ray.dphi;
//
//    k3_ray.dr = ray.dr + 0.5 * d_lambda * k2_d2r;
//    k3_ray.dtheta = ray.dtheta + 0.5 * d_lambda * k2_d2theta;
//    k3_ray.dphi = ray.dphi + 0.5 * d_lambda * k2_d2phi;
//
//    double k3_d2r, k3_d2theta, k3_d2phi;
//    getDerivatives(k3_ray, R_S, k3_d2r, k3_d2theta, k3_d2phi);
//
//    // --- K4 ---
//    Ray k4_ray = ray;
//
//    k4_ray.r = ray.r + d_lambda * k3_ray.dr;
//    k4_ray.theta = ray.theta + d_lambda * k3_ray.dtheta;
//    k4_ray.phi = ray.phi + d_lambda * k3_ray.dphi;
//
//    k4_ray.dr = ray.dr + d_lambda * k3_d2r;
//    k4_ray.dtheta = ray.dtheta + d_lambda * k3_d2theta;
//    k4_ray.dphi = ray.dphi + d_lambda * k3_d2phi;
//
//    double k4_d2r, k4_d2theta, k4_d2phi;
//    getDerivatives(k4_ray, R_S, k4_d2r, k4_d2theta, k4_d2phi);
//
//    // --- Final update: Weighted average ---
//    // Position variables (r, theta, phi)
//    ray.r += (d_lambda / 6.0) * (k1_dr + 2.0 * k2_ray.dr + 2.0 * k3_ray.dr + k4_ray.dr);
//    ray.theta += (d_lambda / 6.0) * (k1_dtheta + 2.0 * k2_ray.dtheta + 2.0 * k3_ray.dtheta + k4_ray.dtheta);
//    ray.phi += (d_lambda / 6.0) * (k1_dphi + 2.0 * k2_ray.dphi + 2.0 * k3_ray.dphi + k4_ray.dphi);
//
//    // Velocity variables (dr, dtheta, dphi)
//    ray.dr += (d_lambda / 6.0) * (k1_d2r + 2.0 * k2_d2r + 2.0 * k3_d2r + k4_d2r);
//    ray.dtheta += (d_lambda / 6.0) * (k1_d2theta + 2.0 * k2_d2theta + 2.0 * k3_d2theta + k4_d2theta);
//    ray.dphi += (d_lambda / 6.0) * (k1_d2phi + 2.0 * k2_d2phi + 2.0 * k3_d2phi + k4_d2phi);
//
//    // Update Cartesian position for rendering/collision
//    ray.UpdateCartesian();
//}
#pragma endregion