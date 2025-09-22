#pragma once
#include "allIncludes.h"
#include "Ray.h"

//void GeoDesic(Ray& ray, double eventHorizonRadius) 
//{
//	double& r = ray.r , phi = ray.phi;
//
//	double& dr = ray.dr, dphi = ray.dphi;
//
//	double& d2r = ray.d2r, d2phi = ray.d2phi;
//
//	d2phi = (1 / r) * dr * dphi;
//	d2r = (-c * c * eventHorizonRadius) / (2 * r * r) + r * dphi * dphi;
//
//	//rate of change of the distance from the blackhole
//	dr += r * dphi * dphi - (c * c * eventHorizonRadius) / (2 * r * r);
//	//rate of change of the angle relative to the blackhole
//	dphi = -2 * dr * dphi / r;	
//}

// calculate second derivatives
void getDerivatives(const Ray& ray, double eventHorizonRadius, double& d2r, double& d2phi) {
    d2r = (-c * c * eventHorizonRadius) / (2.0 * ray.r * ray.r) + ray.r * ray.dphi * ray.dphi;
    d2phi = (-2.0 / ray.r) * ray.dr * ray.dphi;
}

void RK4Step(Ray& ray, double eventHorizonRadius, double d_lambda) {  
   if (ray.r <= eventHorizonRadius) return;  

   // K1: calculate start step
   double k1_dr   = ray.dr;  
   double k1_dphi = ray.dphi;  
   double k1_d2r, k1_d2phi;  
   getDerivatives(ray, eventHorizonRadius, k1_d2r, k1_d2phi);  

   // K2: calculate mid step 
   Ray k2_ray   = ray;  
   k2_ray.r     = ray.r     + 0.5 * d_lambda * k1_dr;  
   k2_ray.phi   = ray.phi   + 0.5 * d_lambda * k1_dphi;  
   k2_ray.dr    = ray.dr    + 0.5 * d_lambda * k1_d2r;  
   k2_ray.dphi  = ray.dphi  + 0.5 * d_lambda * k1_d2phi;  
   double k2_dr = k2_ray.dr;  
   double k2_dphi = k2_ray.dphi;  
   double k2_d2r, k2_d2phi;  
   getDerivatives(k2_ray, eventHorizonRadius, k2_d2r, k2_d2phi);  

   // K3: calculate mid step 
   Ray k3_ray   = ray;  
   k3_ray.r     = ray.r     + 0.5 * d_lambda * k2_dr;  
   k3_ray.phi   = ray.phi   + 0.5 * d_lambda * k2_dphi;  
   k3_ray.dr    = ray.dr    + 0.5 * d_lambda * k2_d2r;  
   k3_ray.dphi  = ray.dphi  + 0.5 * d_lambda * k2_d2phi;  
   double k3_dr = k3_ray.dr;  
   double k3_dphi = k3_ray.dphi;  
   double k3_d2r, k3_d2phi;  
   getDerivatives(k3_ray, eventHorizonRadius, k3_d2r, k3_d2phi);  

   // K4: calculate end step 
   Ray k4_ray   = ray;  
   k4_ray.r     = ray.r     + d_lambda * k3_dr;  
   k4_ray.phi   = ray.phi   + d_lambda * k3_dphi;  
   k4_ray.dr    = ray.dr    + d_lambda * k3_d2r;  
   k4_ray.dphi  = ray.dphi  + d_lambda * k3_d2phi;  
   double k4_dr = k4_ray.dr;  
   double k4_dphi = k4_ray.dphi;  
   double k4_d2r, k4_d2phi;  
   getDerivatives(k4_ray, eventHorizonRadius, k4_d2r, k4_d2phi);  

   // average all steps to get new values and apply them to the original ray
   ray.r    += (d_lambda / 6.0) * (k1_dr    + 2.0 * k2_dr    + 2.0 * k3_dr    + k4_dr);  
   ray.phi  += (d_lambda / 6.0) * (k1_dphi  + 2.0 * k2_dphi  + 2.0 * k3_dphi  + k4_dphi);  
   ray.dr   += (d_lambda / 6.0) * (k1_d2r   + 2.0 * k2_d2r   + 2.0 * k3_d2r   + k4_d2r);  
   ray.dphi += (d_lambda / 6.0) * (k1_d2phi + 2.0 * k2_d2phi + 2.0 * k3_d2phi + k4_d2phi);  
}