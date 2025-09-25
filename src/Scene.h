#pragma once
#include "allIncludes.h"
#include "BlackHole.h"
#include "Camera.h"

using namespace std;
using namespace glm;

class Scene
{
public:
	BlackHole sagittariusA;
	Camera camera;

	Scene(vec3 pos, double mass) : sagittariusA(pos, mass)
	{

	}

	void Update(int width, int height)
	{
		camera.Update();

		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				// shoot out rays for each pixel in the window
				// will probably have to translate each pixel position into 
				// world position :'(

				// then call intersection functions for all objects in the scene 
				// then color these pixels depending on material
			}
		}
	}

	void Render() const
	{

	}
}scene(vec3(0.0f, 0.0f, 0.0f), 8.54e36); //pos of blackhole, mass of sagittarius A*

#pragma region Input detection
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		scene.camera.dragging = true;
		glfwGetCursorPos(window, &scene.camera.lastMouseX, &scene.camera.lastMouseX);
		cout << "DRAGGING \n";
	}
	else
		scene.camera.dragging = false;
}

void mouseMoveCallback(double xpos, double ypos) {
	if (scene.camera.dragging) {
		double deltaX = xpos - scene.camera.lastMouseX;
		double deltaY = ypos - scene.camera.lastMouseY;

		// Update the azimuth and elevation based on the deltas
		scene.camera.azimuth += deltaX * scene.camera.orbitSpeed;
		scene.camera.elevation += deltaY * scene.camera.orbitSpeed;

		// Clamp elevation to prevent flipping
		scene.camera.elevation = glm::clamp(scene.camera.elevation, -glm::pi<float>() / 2.0f + 0.01f, glm::pi<float>() / 2.0f - 0.01f);
	}

	// Always update the last position for the next frame's calculation
	scene.camera.lastMouseX = xpos;
	scene.camera.lastMouseY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// xoffset and yoffset are the scroll offsets
	// xoffset is for horizontal scrolling (not common on a mouse wheel)
	// yoffset is for vertical scrolling
}
#pragma endregion


