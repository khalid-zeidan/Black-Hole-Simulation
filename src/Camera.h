#pragma once
#include "allIncludes.h"

using namespace glm;
using namespace std;

class Camera
{
	vec3 target = vec3(0.0f, 0.0f, 0.0f);
	float radius = 6.34194e10f;
	float minRadius = 1e10f, maxRadius = 1e12f;

	float azimuth = 0.0f;
	float elevation = M_PI / 2.0f;

	float orbitSpeed = 0.01f;
	double zoomSpeed = 25e9f;

	bool dragging = false;
	bool moving = false; // For compute shader optimization
	double lastX = 0.0, lastY = 0.0;

public:

	Camera() 
	{

	}

	void Update()
	{

	}

	vec3 position() const {
		float clampedElevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);

		float x = radius * sin(clampedElevation) * cos(azimuth);
		float y = radius * cos(clampedElevation);
		float z = radius * sin(clampedElevation) * sin(azimuth);

		return vec3(x, y, z);
	}
};

