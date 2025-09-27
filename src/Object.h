#pragma once
#include "allIncludes.h"

using namespace std;
using namespace glm;

struct Object 
{
public:
	vec3 position;
	float radius;
	vec3 color; // rgba

	bool Intercept(float x, float y, float z) const
	{
		float dx = x - position.x;
		float dy = y - position.y;
		float dz = z - position.z;
		float distanceSquared = dx * dx + dy * dy + dz * dz;
		return distanceSquared <= (radius * radius);
	}
};

