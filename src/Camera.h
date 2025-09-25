#pragma once
#include "allIncludes.h"
#include "Engine.h"

using namespace glm;
using namespace std;

class Camera
{
	// center of black hole
	vec3 target = vec3(0.0f, 0.0f, 0.0f);

	// used for zoom
	float radius = 6.34194e10f;
	float minRadius = 1e10f, maxRadius = 1e12f;

public:

	float azimuth = 0.0f;			// point from left to right on the sphere 360 degrees
	float elevation = M_PI / 2.0f;  // point from up to down on the sphere 180 degrees only

	float sensitivity = 0.01f;
	double zoomSensitivity = 25e9f;

	double lastMouseX = 0.0f, lastMouseY = 0.0f;
	bool dragging = false;
	bool firstMouse = true;

	Camera() {}

	void Update()
	{
		cout << "Azimuth = " << azimuth << endl;
		cout << "Elevation = " << elevation << endl;
		cout << "Zoom = " << radius << endl;
	}

	// calculates position in world space based on elevation, zoom and azimuth
	vec3 calculatePosition() const {
		float clampedElevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);

		float x = radius * sin(clampedElevation) * cos(azimuth);
		float y = radius * cos(clampedElevation);
		float z = radius * sin(clampedElevation) * sin(azimuth);

		return vec3(x, y, z);
	}

	void Rotate(float deltaX, float deltaY) {
		azimuth += deltaX * sensitivity;
		elevation -= deltaY * sensitivity;

		elevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);
	}

	void Zoom(float delta) {
		radius -= delta * zoomSensitivity;

		// to avoid clipping into the black hole
		radius = clamp(radius, minRadius, maxRadius);
	}

	mat4 GetViewMatrix() const {
		vec3 position = calculatePosition();
		vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

		return lookAt(position, target, upVector);
	}
};
