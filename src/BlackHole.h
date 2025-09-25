#pragma once
#include "allIncludes.h"
using namespace std;
using namespace glm;

class BlackHole
{
public:
	vec3 position;
	double mass;
	double eventHorizonRadius;

	BlackHole(vec3 pos, double m) : position(pos), mass(m) {
		eventHorizonRadius = (2 * G * mass) / (c * c); // Event Horizon radius
	}

	// checks if the point of ray intersects the black hole
	// should then color that pixel black
	bool Intercept(double x, double y, double z) {
		//uses the distance formula sqrt((x2 - x1)^2 + (y2 - y1)^2 + (z2 - z1)^2) = distance

		double dx = x - position.x;
		double dy = y - position.y;
		double dz = z - position.z;
		double distanceSquared = dx * dx + dy * dy + dz * dz;
		return distanceSquared < (eventHorizonRadius * eventHorizonRadius);
	}
};

