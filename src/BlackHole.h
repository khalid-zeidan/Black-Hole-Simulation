#pragma once
#include "allIncludes.h"
using namespace std;
using namespace glm;

class BlackHole
{
public:
	vec3 position;
	float mass;
	float R_S; // event horizon radius

	BlackHole(vec3 pos, float m) : position(pos), mass(m) {
		R_S = (2 * G * mass) / (c * c); // Event Horizon radius
	}

	// checks if the point of ray intersects the black hole
	// should then color that pixel black
	bool Intercept(float x, float y, float z) const {
		//uses the distance formula sqrt((x2 - x1)^2 + (y2 - y1)^2 + (z2 - z1)^2) = distance

		float dx = x - position.x;
		float dy = y - position.y;
		float dz = z - position.z;
		float distanceSquared = dx * dx + dy * dy + dz * dz;
		return distanceSquared <= (R_S * R_S);
	}

	bool Intercept(float r) const {
		return r <= R_S;
	}
};

