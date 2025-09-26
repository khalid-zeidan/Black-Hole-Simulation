#pragma once
#include "allIncludes.h"
#include "Object.h"
#include "BlackHole.h"
#include "Camera.h"

using namespace std;
using namespace glm;

struct Ray 
{
	vec3 cartesianPosition;

	double r, phi, theta;    //position in spherical coord
	double dr, dphi, dtheta; //velocity based on affine parameter (dlambda)

	double E; // Energy
	double L; // Angular Momentum

	//void UpdateCartesian()
	//{
	//	cartesianPosition.x = r * sin(theta) * cos(phi);
	//	cartesianPosition.y = r * sin(theta) * sin(phi);
	//	cartesianPosition.z = r * cos(theta);
	//}

	void CartesianToPolar(const vec3& cartesian) 
	{
		r = length(cartesian);
		theta = acos(cartesian.z / r);
		phi = atan(cartesian.y, cartesian.x);
	}
};

class RayTracer
{
	const double dLambda = 1e-1;
	const double maxSteps = 4000;

public:
	RayTracer(){}

	static Ray GetInitialRay(const Camera& camera, int x, int y, int WIDTH, int HEIGHT) 
	{
		Ray initialRay;

		double normalizedX = (2.0f * x / WIDTH) - 1.0f;
		double normalizedY = 1 - (2.0f * y / HEIGHT);

		vec3 rayDir = normalize(camera.front +
								camera.right * (float)normalizedX +
								camera.up * (float)normalizedY);

		initialRay.cartesianPosition = camera.calculatePosition();
		initialRay.CartesianToPolar(initialRay.cartesianPosition);

		// calculate initial E, L
		// calculate initial dr, dphi, dtheta
		// using ray direction

		return initialRay;
	}

	vec3 TraceAndGetColor(Ray& initialRay,const BlackHole& blackHole, const vector<Object>& Objects) 
	{
		Ray currentRay = initialRay;

		for (int i = 0; i < maxSteps; i++)
		{
			// intercept blackhole
			if (blackHole.Intercept(currentRay.r))
			{
				return vec3(255, 0, 0); //return red for now (color of blackhole)
			}

			// intercept objects
			for (const auto& object: Objects)
			{
				double x = currentRay.cartesianPosition.x;
				double y = currentRay.cartesianPosition.y;
				double z = currentRay.cartesianPosition.z;

				if (object.Intercept(x, y, z))
				{
					return object.color; //return object's color
				}
			}

			RK4STEP(currentRay, blackHole.R_S);
		}

		return vec3(255, 0, 255); // return magenta if nothing is hit (will return black soon)
	}

private:
	void GetDerivatives(Ray& ray) {

	}

	void RK4STEP(Ray& ray, double R_S) 
	{

	}
};