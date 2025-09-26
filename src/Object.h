#pragma once
#include "allIncludes.h"

using namespace std;
using namespace glm;

struct Object 
{
	vec3 position;
	float radius;

public:
	vec3 color; // rgba

	bool Intercept(double x, double y, double z) const
	{
		double dx = x - position.x;
		double dy = y - position.y;
		double dz = z - position.z;
		double distanceSquared = dx * dx + dy * dy + dz * dz;
		return distanceSquared < (radius * radius);
	}

	/// when ray intersect object it will color that ray's pixel using the
	/// color variable on each object
};

