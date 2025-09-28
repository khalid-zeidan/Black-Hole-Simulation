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

	float r, phi, theta;    //position in spherical coord
	float dr, dphi, dtheta; //velocity based on affine parameter (dlambda)

	float E; // Energy
	float L; // Angular Momentum

	void SphericalToCartesian()
	{
		cartesianPosition.x = r * sin(theta) * cos(phi);
		cartesianPosition.y = r * sin(theta) * sin(phi);
		cartesianPosition.z = r * cos(theta);
	}

	void CartesianToSpherical(const vec3& cartesian) 
	{
		r = length(cartesian);
		if (r < 1e-10) { // Avoid division by zero at the origin
			theta = 0.0;
			phi = 0.0;
			return;
		}
		theta = acos(cartesian.z / r);
		phi = atan(cartesian.y, cartesian.x);
	}
};

class RayTracer
{
public:
	const float dLambda = 1e7;
	const float maxSteps = 20000;
	const float escapeRadius = 1e17;

	RayTracer(){}

	static Ray GetInitialRay(const Camera& camera, int pixelX, int pixelY, int WIDTH, int HEIGHT, const BlackHole& blackHole)
	{
		Ray initialRay;
		vec3 cameraPos = camera.calculatePosition();

		float ndcX = ((pixelX + 0.5f) / (float)WIDTH) * 2.0f - 1.0f;
		float ndcY = ((pixelY + 0.5f) / (float)HEIGHT) * 2.0f - 1.0f;

		float aspect = (float)WIDTH / (float)HEIGHT;
		float tanHalfFov = tanf(0.5f * radians(60.0f));

		float px = ndcX * aspect * tanHalfFov;
		float py = -ndcY * tanHalfFov; // flip Y

		vec3 forward = normalize(camera.target - cameraPos);
		vec3 worldUp = vec3(0.0f, 1.0f, 0.0f);

		// Handle edge case: forward nearly parallel to worldUp
		if (fabs(dot(forward, worldUp)) > 0.999f)
			worldUp = vec3(0.0f, 0.0f, 1.0f);

		vec3 right = normalize(cross(forward, worldUp));
		vec3 up = normalize(cross(right, forward));

		vec3 dir = normalize(forward + px * right + py * up);
		initialRay.direction = dir;
		initialRay.cartesianPosition = cameraPos;

		vec3 relPos = camera.calculatePosition() - blackHole.position;
		initialRay.r = length(relPos);
		initialRay.theta = acos(relPos.z / initialRay.r);
		initialRay.phi = atan2(relPos.y, relPos.x);

		float dx = dir.x, dy = dir.y, dz = dir.z;

		initialRay.dr = sin(initialRay.theta) * cos(initialRay.phi) * dx + sin(initialRay.theta) * sin(initialRay.phi) * dy + cos(initialRay.theta) * dz;
		initialRay.dtheta = (cos(initialRay.theta) * cos(initialRay.phi) * dx + cos(initialRay.theta) * sin(initialRay.phi) * dy - sin(initialRay.theta) * dz) / initialRay.r;
		initialRay.dphi = (-sin(initialRay.phi) * dx + cos(initialRay.phi) * dy) / (initialRay.r * sin(initialRay.theta));

		initialRay.L = initialRay.r * initialRay.r * sin(initialRay.theta) * initialRay.dphi;

		float f = 1.0 - blackHole.R_S / initialRay.r;
		float dt_dL = sqrt((initialRay.dr * initialRay.dr) / f + initialRay.r * initialRay.r * (initialRay.dtheta * initialRay.dtheta + sin(initialRay.theta) * sin(initialRay.theta) * initialRay.dphi * initialRay.dphi));

		initialRay.E = f * dt_dL;

		return initialRay;
	}

	vec3 TraceAndGetColor(Ray& initialRay, const BlackHole& blackHole, const vector<Object>& Objects) 
	{
		Ray currentRay = initialRay;

		for (int i = 0; i < maxSteps; i++)
		{
			//LinearSTEP(currentRay);

			RK4STEP(currentRay, blackHole.R_S);

			//escape
			if (currentRay.r > escapeRadius)
				return vec3(0, 0, 0);

			float x = currentRay.cartesianPosition.x;
			float y = currentRay.cartesianPosition.y;
			float z = currentRay.cartesianPosition.z;

			//intercept blackhole
			if (blackHole.Intercept(currentRay.r))
			{
				return vec3(255, 255, 255); 
			}

			//intercept objects
			for (const auto& object: Objects)
			{
				if (object.Intercept(x, y, z))
				{
					return object.color; //return object's color
				}
			}
		}

		return vec3(0, 0, 0);
	}

private:
	void LinearSTEP(Ray& ray) 
	{
		ray.cartesianPosition += ray.direction * (float)dLambda;

		ray.r = length(ray.cartesianPosition);
		ray.theta = acos(ray.cartesianPosition.z / ray.r);
		ray.phi = atan2(ray.cartesianPosition.y, ray.cartesianPosition.x);
	}

	void GetDerivatives(const Ray& ray, float R_S, float& d2r, float& d2theta, float& d2phi) {

		float r = ray.r, theta = ray.theta;
		float dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;
		float rs = R_S;

		float f = 1.0 - rs / r;
		float dt_dL = ray.E / f;

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

	void RK4STEP(Ray& ray, float R_S)
	{
		// --- K1 ---
		Ray k1_ray = ray;
		float k1_d2r, k1_d2theta, k1_d2phi;
		GetDerivatives(k1_ray, R_S, k1_d2r, k1_d2theta, k1_d2phi);

		// --- K2 ---
		Ray k2_ray = ray;

		k2_ray.r = ray.r + 0.5 * dLambda * k1_ray.dr;
		k2_ray.theta = ray.theta + 0.5 * dLambda * k1_ray.dtheta;
		k2_ray.phi = ray.phi + 0.5 * dLambda * k1_ray.dphi;

		k2_ray.dr = ray.dr + 0.5 * dLambda * k1_d2r;
		k2_ray.dtheta = ray.dtheta + 0.5 * dLambda * k1_d2theta;
		k2_ray.dphi = ray.dphi + 0.5 * dLambda * k1_d2phi;

		float k2_d2r, k2_d2theta, k2_d2phi;
		GetDerivatives(k2_ray, R_S, k2_d2r, k2_d2theta, k2_d2phi);

		// --- K3 ---
		Ray k3_ray = ray;

		k3_ray.r = ray.r + 0.5 * dLambda * k2_ray.dr;
		k3_ray.theta = ray.theta + 0.5 * dLambda * k2_ray.dtheta;
		k3_ray.phi = ray.phi + 0.5 * dLambda * k2_ray.dphi;

		k3_ray.dr = ray.dr + 0.5 * dLambda * k2_d2r;
		k3_ray.dtheta = ray.dtheta + 0.5 * dLambda * k2_d2theta;
		k3_ray.dphi = ray.dphi + 0.5 * dLambda * k2_d2phi;

		float k3_d2r, k3_d2theta, k3_d2phi;
		GetDerivatives(k3_ray, R_S, k3_d2r, k3_d2theta, k3_d2phi);

		// --- K4 ---
		Ray k4_ray = ray;

		k4_ray.r = ray.r + dLambda * k3_ray.dr;
		k4_ray.theta = ray.theta + dLambda * k3_ray.dtheta;
		k4_ray.phi = ray.phi + dLambda * k3_ray.dphi;

		k4_ray.dr = ray.dr + dLambda * k3_d2r;
		k4_ray.dtheta = ray.dtheta + dLambda * k3_d2theta;
		k4_ray.dphi = ray.dphi + dLambda * k3_d2phi;

		float k4_d2r, k4_d2theta, k4_d2phi;
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