#pragma once
#include "allIncludes.h"

using namespace glm;
using namespace std;

class Camera
{
	// used for zoom
	float radius = 1.1e11f;
	float minRadius = 1.5e10f, maxRadius = 15e10f;

public:
	vec3 target = vec3(0.0f, 0.0f, 0.0f);

	float azimuth = 2.86;// 0.0f;			 // point from left to right on the sphere 360 degrees
	float elevation = 1.54f;//-M_PI / 1.5f;  // point from up to down on the sphere 180 degrees only

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
		float clampedElevation = clamp(elevation, 0.01f, float(M_PI) - 0.01f);

		float x = radius * sin(clampedElevation) * cos(azimuth);
		float y = radius * cos(clampedElevation);
		float z = radius * sin(clampedElevation) * sin(azimuth);

		return vec3(x, y, z);
	}

	void Rotate(float deltaX, float deltaY) {
		azimuth += deltaX * sensitivity;
		elevation += deltaY * sensitivity;

		elevation = clamp(elevation, 0.01f, float(M_PI) - 0.01f);
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
		return perspective(radians(60.0f), width / height, minRadius * 0.1f, maxRadius * 100.0f);
		// FOV, aspect ratio, near & far plane
	}

	void LogCameraData() {
		cout << "AZIMUTH: " << azimuth << endl;
		cout << "ELEVATION: " << elevation << endl;
		cout << "RADIUS: " << radius << endl;
	}
}camera;

#pragma region Camera Input detection
//checks if mouse buttons are pressed
void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		camera.dragging = true;
	}
	else
	{
		camera.dragging = false;
		camera.firstMouse = true;
	}
}

// checks mouse move positions
void mouseMoveCallBack(GLFWwindow* window, double xpos, double ypos)
{
	if (camera.firstMouse)
	{
		camera.lastMouseX = xpos;
		camera.lastMouseY = ypos;
		camera.firstMouse = false;
	}

	float deltaX = (float)(xpos - camera.lastMouseX);
	float deltaY = (float)(camera.lastMouseY - ypos);

	camera.lastMouseX = xpos;
	camera.lastMouseY = ypos;

	if (camera.dragging) {
		// Pass the delta values to the camera's rotate method
		camera.Rotate(deltaX, deltaY);
	}
}

//checks mouse scroll 
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.Zoom(yoffset);
}
#pragma endregion