#pragma once
#include "allIncludes.h"
#include "BlackHole.h"
#include "Camera.h"
#include "Object.h"

using namespace std;
using namespace glm;

class Scene
{
public:
	BlackHole sagittariusA;
	Camera camera;

	// position, color, radius
	vector<Object> objectsData = {
		{vec3(4e11f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), 4e5f},
		{vec3(0.0f, 4e11f, 4e11f), vec4(0.4f, 0.2f, 1.0f, 1.0f), 4e5f}
	}; 

	Scene(vec3 pos, double mass) : sagittariusA(pos, mass) {}

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

#pragma region Camera Input detection
void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		scene.camera.dragging = true;
		cout << "DRAGGING \n";
	}
	else 
	{
		scene.camera.dragging = false;
		scene.camera.firstMouse = true;
	}
}

void mouseMoveCallBack(GLFWwindow* window, double xpos, double ypos)
{
	if (scene.camera.firstMouse) 
	{
		scene.camera.lastMouseX = xpos;
		scene.camera.lastMouseY = ypos;
		scene.camera.firstMouse = false;
	}

	float deltaX = (float)(xpos - scene.camera.lastMouseX);
	float deltaY = (float)(scene.camera.lastMouseY - ypos);

	scene.camera.lastMouseX = xpos;
	scene.camera.lastMouseY = ypos;

	if (scene.camera.dragging) {
		// Pass the delta values to the camera's rotate method
		scene.camera.Rotate(deltaX, deltaY);
	}
}

void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
	scene.camera.Zoom(yoffset);
}
#pragma endregion


