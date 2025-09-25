#include "allIncludes.h"
#include "Engine.h"
#include "BlackHole.h"
#include "Ray.h"
#include "Physics.h"
#include "Scene.h"

using namespace glm;
using namespace std;

Engine engine;

#pragma region function declaration
void start();
void Update();
void Draw();
#pragma endregion

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
	// for camera inputs
	glfwSetMouseButtonCallback(engine.window, mouseButtonCallBack);
	glfwSetCursorPosCallback(engine.window, mouseMoveCallBack);
	glfwSetScrollCallback(engine.window, scrollCallBack);
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