#pragma once
#include "allIncludes.h"

using namespace std;
using namespace glm;

struct Object 
{
public:
	vec4 positionRadius;
	vec4 color; // rgba
	vec4 padding;

	bool Intercept(float x, float y, float z) const
	{
		float dx = x - positionRadius.x;
		float dy = y - positionRadius.y;
		float dz = z - positionRadius.z;
		float distanceSquared = dx * dx + dy * dy + dz * dz;
		return distanceSquared <= (positionRadius.w * positionRadius.w);
	}
};

