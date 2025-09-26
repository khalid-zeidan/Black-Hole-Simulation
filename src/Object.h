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

	bool Intercept(float x, float y, float z) const
	{
		float dx = x - position.x;
		float dy = y - position.y;
		float dz = z - position.z;
		float distanceSquared = dx * dx + dy * dy + dz * dz;
		return distanceSquared <= (radius * radius);
	}

	/// when ray intersect object it will color that ray's pixel using the
	/// color variable on each object
};

