#pragma once
#include "allIncludes.h"
#include "Object.h"
#include "BlackHole.h"
#include "Camera.h"

using namespace std;
using namespace glm;

struct Ray 
{
	vec3 cartesianPosition, direction;

	double r, phi, theta;    //position in spherical coord
	double dr, dphi, dtheta; //velocity based on affine parameter (dlambda)

	double E; // Energy
	double L; // Angular Momentum

	void UpdateCartesian()
	{
		cartesianPosition.x = r * sin(theta) * cos(phi);
		cartesianPosition.y = r * sin(theta) * sin(phi);
		cartesianPosition.z = r * cos(theta);
	}

	void CartesianToPolar(const vec3& cartesian) 
	{
		r = length(cartesian);
		//if (r < 1e-10) { // Avoid division by zero at the origin
		//	theta = 0.0;
		//	phi = 0.0;
		//	return;
		//}
		theta = acos(cartesian.z / r);
		phi = atan(cartesian.y, cartesian.x);
	}
};

class RayTracer
{
	const double dLambda = 1e7;
	const double maxSteps = 10000;

public:
	RayTracer(){}

	static Ray GetInitialRay(const Camera& camera, int x, int y, int WIDTH, int HEIGHT, const BlackHole& blackHole) 
	{
		Ray initialRay;

		double normalizedX = (2.0f * x / WIDTH) - 1.0f;
		double normalizedY = 1 - (2.0f * y / HEIGHT);

		vec3 fwd = normalize(camera.target - camera.calculatePosition());
		vec3 up = vec3(0, 1, 0); // y axis is up, so disk is in x-z plane
		vec3 right = normalize(cross(fwd, up));
		up = cross(right, fwd);

		vec3 rayDir = normalize(fwd +
								right * (float)normalizedX +
								up	  * (float)normalizedY);

		initialRay.cartesianPosition = camera.calculatePosition();
		initialRay.CartesianToPolar(initialRay.cartesianPosition);
		initialRay.direction = rayDir;

		// Convert cartesian velocity to spherical velocity
		float dx = rayDir.x, dy = rayDir.y, dz = rayDir.z;

		initialRay.dr = sin(initialRay.theta) * cos(initialRay.phi) * dx + sin(initialRay.theta) * sin(initialRay.phi) * dy + cos(initialRay.theta) * dz;
		initialRay.dtheta = (cos(initialRay.theta) * cos(initialRay.phi) * dx + cos(initialRay.theta) * sin(initialRay.phi) * dy - sin(initialRay.theta) * dz) / initialRay.r;
		initialRay.dphi = (-sin(initialRay.phi) * dx + cos(initialRay.phi) * dy) / (initialRay.r * sin(initialRay.theta));

		// --- VITAL FIX: SCALE UP MOMENTUM COMPONENTS ---
		// The components must be scaled by the characteristic length (R_S) to give the ray 
		// enough energy/momentum to curve significantly.
		double scaleFactor = blackHole.R_S;

		initialRay.dr *= scaleFactor;
		initialRay.dtheta *= scaleFactor;
		initialRay.dphi *= scaleFactor;
		// ---------------------------------------------

		initialRay.L = initialRay.r * initialRay.r * sin(initialRay.theta) * initialRay.dphi;
		float f = 1.0 - blackHole.R_S / initialRay.r;

		// This now calculates the correct dt/dlambda from the scaled components
		float dt_dL = sqrt((initialRay.dr * initialRay.dr) / f + initialRay.r * initialRay.r * (initialRay.dtheta * initialRay.dtheta + sin(initialRay.theta) * sin(initialRay.theta) * initialRay.dphi * initialRay.dphi));

		initialRay.E = f * dt_dL;

		return initialRay;
	}

	vec3 TraceAndGetColor(Ray& initialRay,const BlackHole& blackHole, const vector<Object>& Objects) 
	{
		Ray currentRay = initialRay;

		for (int i = 0; i < maxSteps; i++)
		{
			//escape
			if (currentRay.r > 1e12)
				return vec3(0, 0, 0);

			double x = currentRay.cartesianPosition.x;
			double y = currentRay.cartesianPosition.y;
			double z = currentRay.cartesianPosition.z;

			// intercept blackhole
			if (blackHole.Intercept(x, y, z))
			{
				return vec3(255, 255, 255); //return red for now (color of blackhole)
			}

			//intercept objects
			for (const auto& object: Objects)
			{
				if (object.Intercept(x, y, z))
				{
					return object.color; //return object's color
				}
			}

			//linear rays no gravity lensing
			//currentRay.cartesianPosition += (float)dLambda * currentRay.direction;
			RK4STEP(currentRay, blackHole.R_S);
			currentRay.UpdateCartesian();
		}

		return vec3(0, 0, 0); // return magenta if nothing is hit (will return black soon)
	}

private:
	void GetDerivatives(const Ray& ray, double R_S, double& d2r, double& d2theta, double& d2phi) {

		double r = ray.r, theta = ray.theta;
		double dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;
		double rs = R_S;

		double f = 1.0 - rs / r;
		double dt_dL = ray.E / f;

		// Avoid division by zero when approaching the event horizon
		if (f == 0.0) {
			d2r = d2theta = d2phi = 0.0;
			return;
		}

		// These are the geodesic equations for a Schwarzschild metric
		d2r = -(rs / (2.0 * r * r)) * f * dt_dL * dt_dL
			+ (rs / (2.0 * r * r * f)) * dr * dr
			+ r * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi);

		d2theta = -2.0 * dr * dtheta / r
			+ sin(theta) * cos(theta) * dphi * dphi;

		double sin_theta = sin(theta);
		// Avoid pole singularity
		if (abs(sin_theta) < 1e-10) sin_theta = 1e-10;

		d2phi = -2.0 * dr * dphi / r
			- 2.0 * cos(theta) / sin_theta * dtheta * dphi;
	}

	// Full Runge-Kutta 4th Order numerical integration
	void RK4STEP(Ray& ray, double R_S)
	{
		// --- K1 ---
		Ray k1_ray = ray;
		double k1_d2r, k1_d2theta, k1_d2phi;
		GetDerivatives(k1_ray, R_S, k1_d2r, k1_d2theta, k1_d2phi);

		// --- K2 ---
		Ray k2_ray = ray;

		k2_ray.r = ray.r + 0.5 * dLambda * k1_ray.dr;
		k2_ray.theta = ray.theta + 0.5 * dLambda * k1_ray.dtheta;
		k2_ray.phi = ray.phi + 0.5 * dLambda * k1_ray.dphi;

		k2_ray.dr = ray.dr + 0.5 * dLambda * k1_d2r;
		k2_ray.dtheta = ray.dtheta + 0.5 * dLambda * k1_d2theta;
		k2_ray.dphi = ray.dphi + 0.5 * dLambda * k1_d2phi;

		double k2_d2r, k2_d2theta, k2_d2phi;
		GetDerivatives(k2_ray, R_S, k2_d2r, k2_d2theta, k2_d2phi);

		// --- K3 ---
		Ray k3_ray = ray;

		k3_ray.r = ray.r + 0.5 * dLambda * k2_ray.dr;
		k3_ray.theta = ray.theta + 0.5 * dLambda * k2_ray.dtheta;
		k3_ray.phi = ray.phi + 0.5 * dLambda * k2_ray.dphi;

		k3_ray.dr = ray.dr + 0.5 * dLambda * k2_d2r;
		k3_ray.dtheta = ray.dtheta + 0.5 * dLambda * k2_d2theta;
		k3_ray.dphi = ray.dphi + 0.5 * dLambda * k2_d2phi;

		double k3_d2r, k3_d2theta, k3_d2phi;
		GetDerivatives(k3_ray, R_S, k3_d2r, k3_d2theta, k3_d2phi);

		// --- K4 ---
		Ray k4_ray = ray;

		k4_ray.r = ray.r + dLambda * k3_ray.dr;
		k4_ray.theta = ray.theta + dLambda * k3_ray.dtheta;
		k4_ray.phi = ray.phi + dLambda * k3_ray.dphi;

		k4_ray.dr = ray.dr + dLambda * k3_d2r;
		k4_ray.dtheta = ray.dtheta + dLambda * k3_d2theta;
		k4_ray.dphi = ray.dphi + dLambda * k3_d2phi;

		double k4_d2r, k4_d2theta, k4_d2phi;
		GetDerivatives(k4_ray, R_S, k4_d2r, k4_d2theta, k4_d2phi);

		// --- Final update
		ray.r += (dLambda / 6.0) * (k1_ray.dr + 2.0 * k2_ray.dr + 2.0 * k3_ray.dr + k4_ray.dr);
		ray.theta += (dLambda / 6.0) * (k1_ray.dtheta + 2.0 * k2_ray.dtheta + 2.0 * k3_ray.dtheta + k4_ray.dtheta);
		ray.phi += (dLambda / 6.0) * (k1_ray.dphi + 2.0 * k2_ray.dphi + 2.0 * k3_ray.dphi + k4_ray.dphi);

		ray.dr += (dLambda / 6.0) * (k1_d2r + 2.0 * k2_d2r + 2.0 * k3_d2r + k4_d2r);
		ray.dtheta += (dLambda / 6.0) * (k1_d2theta + 2.0 * k2_d2theta + 2.0 * k3_d2theta + k4_d2theta);
		ray.dphi += (dLambda / 6.0) * (k1_d2phi + 2.0 * k2_d2phi + 2.0 * k3_d2phi + k4_d2phi);
	}
};