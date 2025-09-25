#pragma once
#include "allIncludes.h"

using namespace std;
using namespace glm;

class Ray
{
public:
	// cartesian coordinates
	double x, y, z;
	// polar coordinates
	double r	, phi	, theta;
	double dr	, dphi	, dtheta;
	double d2r	, d2phi	, d2theta;

	double E; // Energy
	double L; // Angular Momentum

	vec3 direction;
	double eventHorizionRadius;

	Ray(vec3 pos, vec3 dir, double EHRadius) : x(pos.x), y(pos.y), z(pos.z), direction(dir), eventHorizionRadius(EHRadius)
	{
		r = length(pos);
		theta = acos(pos.z / r);
		phi = atan(pos.y, pos.x);

		//avoids dividing by 0
		double r_sin_theta = r * sin(theta);
		if (r_sin_theta == 0.0) r_sin_theta = 1e-10;

		double dx = dir.x, dy = dir.y, dz = dir.z;

		//init coord transformation for velocity
		dr = sin(theta) * cos(phi) * dx + sin(theta) * sin(phi) * dy + cos(theta) * dz;
		dtheta = (cos(theta) * cos(phi) * dx + cos(theta) * sin(phi) * dy - sin(theta) * dz) / r;
		dphi = (-sin(phi) * dx + cos(phi) * dy) / r_sin_theta;

		L = r_sin_theta * r_sin_theta * dphi;
		double f = 1.0 - eventHorizionRadius / r;

		double dt_dL_sq = (dr * dr) / f + r * r * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi);
		double dt_dL = sqrt(dt_dL_sq / f);

		double E_sq = (dr * dr) + f * r * r * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi);
		E = sqrt(E_sq);
	}

	// RK4 step function then update the position
	void UpdateCartesian() 
	{
		x = r * sin(theta) * cos(phi);
		y = r * sin(theta) * sin(phi);
		z = r * cos(theta);
	}

	bool intercept() {
		return r <= eventHorizionRadius;
	}
};
