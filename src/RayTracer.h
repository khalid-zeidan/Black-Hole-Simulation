#pragma once
#include "allIncludes.h"
#include "Object.h"
#include "BlackHole.h"

using namespace std;
using namespace glm;

struct Ray 
{
	vec3 cartesianPosition;

	double r, phi, theta;    //position in spherical coord
	double dr, dphi, dtheta; //velocity based on affine parameter (dlambda)

	double E; // Energy
	double L; // Angular Momentum
};

class RayTracer
{
	const double dLambda = 1e-1;
	const double maxSteps = 4000;

public:
	RayTracer(){}

	vec3 TraceAndGetColor(Ray& initialRay,const BlackHole& blackHole, const vector<Object>& Objects) 
	{
		Ray currentRay = initialRay;

		for (int i = 0; i < maxSteps; i++)
		{
			if (blackHole.Intercept(currentRay.r))
			{
				return vec3(255, 0, 0); //return red for now (color of blackhole)
			}

			for (const auto& object: Objects)
			{
				if (distance(currentRay.cartesianPosition, object.position) < blackHole.R_S)
				{
					return object.color; //return object's color
				}
			}

			RK4STEP(currentRay, blackHole.R_S);
		}

		return vec3(255, 0, 255); // return magenta if nothing is hit
	}

private:
	void GetDerivatives(Ray& ray) {

	}

	void RK4STEP(Ray& ray, double R_S) 
	{

	}
};