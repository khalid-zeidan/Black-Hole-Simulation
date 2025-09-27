#pragma once
#include "allIncludes.h"

using namespace glm;
using namespace std;

class Camera
{
	// used for zoom
	float radius = 6.34194e10f;
	float minRadius = 1e10f, maxRadius = 9.8e10f;

public:
	vec3 target = vec3(0.0f, 0.0f, 0.0f);

	float azimuth = 0.0f;			 // point from left to right on the sphere 360 degrees
	float elevation = -M_PI / 1.5f;  // point from up to down on the sphere 180 degrees only

private:
	float sensitivity = 0.01f;
	float zoomSensitivity = 25e9f;

public:
	float lastMouseX = 0.0f, lastMouseY = 0.0f;
	bool dragging = false;
	bool firstMouse = true;

	Camera() {}

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

	mat4 GetProjectionMatrix(float width, float height) const {
		return perspective(radians(60.0f), width / height, minRadius * 0.1f, maxRadius * 1000.0f);
		// FOV, aspect ratio, near & far plane
	}
};
