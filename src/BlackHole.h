#pragma once
#include "allIncludes.h"
using namespace std;
using namespace glm;

class BlackHole
{
public:
	vec2 position;
	double mass;
	double eventHorizonRadius;

	BlackHole(vec2 pos, double m) : position(pos), mass(m) {
		eventHorizonRadius = (2 * G * mass) / (c * c); // Event Horizon radius
	}

	void Draw() {
		glBegin(GL_TRIANGLE_FAN);

		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(position.x, position.y);
		int numSegments = 100;

		for (int i = 0; i <= numSegments; i++)
		{
			float angle = (2 * M_PI * i) / numSegments;
			float x = position.x + (eventHorizonRadius * cos(angle));
			float y = position.y + (eventHorizonRadius * sin(angle));
			glVertex2f(x, y);
		}
		glEnd();
	}
};

