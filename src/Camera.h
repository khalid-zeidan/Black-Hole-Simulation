#pragma once
#include "allIncludes.h"
#include "Engine.h"

using namespace glm;
using namespace std;

class Camera
{
	vec3 target = vec3(0.0f, 0.0f, 0.0f);

	float radius = 6.34194e10f;
	float minRadius = 1e10f, maxRadius = 1e12f;

public:

	float azimuth = 0.0f; // point from left to right on the sphere 360 degrees
	float elevation = M_PI / 2.0f; // point from up to down on the sphere 180 degrees only

	float orbitSpeed = 0.01f;
	double zoomSpeed = 25e9f;



	double lastMouseX = 0.0f, lastMouseY = 0.0f;
	bool dragging = false;

	Camera()
	{
		
	}

	void Update()
	{
		// checks for mouse and kboard inputs and moves accordingly
	}

	vec3 position() const {
		float clampedElevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);

		float x = radius * sin(clampedElevation) * cos(azimuth);
		float y = radius * cos(clampedElevation);
		float z = radius * sin(clampedElevation) * sin(azimuth);

		return vec3(x, y, z);
	}
};
