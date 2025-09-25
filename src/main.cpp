#include "allIncludes.h"
#include "Engine.h"
#include "BlackHole.h"
#include "Ray.h"
#include "Physics.h"
#include "Scene.h"

using namespace glm;
using namespace std;

Engine engine;

void start();
void Update();
void Draw();

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double yoffset);

int main()
{
	start();

	while (!glfwWindowShouldClose(engine.window))
	{
		Update();
		Draw();

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void start()
{
	glfwSetMouseButtonCallback(engine.window, mouseButtonCallback);
	glfwSetScrollCallback(engine.window, scroll_callback);
}

void Update() 
{
	scene.Update(engine.WIDTH, engine.HEIGHT);
}

void Draw() 
{
	engine.Clear();

	scene.Render();
}