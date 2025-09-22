#pragma once
#include "allIncludes.h"
using namespace std;
using namespace glm;

class Ray
{
public:
	// cartesian coordinates
	double x, y;
	// polar coordinates
	double r, phi;

	vec2 direction;
	vector<vec2> trail;

	Ray(vec2 pos, vec2 dir) : x(pos.x), y(pos.y), direction(dir) {}

	void Draw() {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(1.0f);

		size_t N = trail.size();
		if (N < 2) return;


		glBegin(GL_LINE_STRIP);	

		for (size_t i = 0; i < N; ++i)
		{
			float alpha = float(i) / float(N-1);
			glColor4f(1.0f, 1.0f, 1.0f, std::max(alpha, 0.05f));
			glVertex2f(trail[i].x, trail[i].y);
		}
		glEnd();
	}

	void Step() {
		x += direction.x * c;
		y += direction.y * c;

		trail.push_back({x, y});
	}
};

