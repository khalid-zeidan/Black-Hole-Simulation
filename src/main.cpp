#include "allIncludes.h"
#include "Scene.h"

using namespace glm;
using namespace std;

Engine& eng = scene.engine;

#pragma region function declaration
void start();
void Update();
void Draw();
#pragma endregion

int main()
{
	start();

	while (!glfwWindowShouldClose(eng.window))
	{
		Update();
		Draw();

		glfwSwapBuffers(eng.window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void start()
{
	// for camera inputs
	glfwSetMouseButtonCallback(eng.window, mouseButtonCallBack);
	glfwSetCursorPosCallback(eng.window, mouseMoveCallBack);
	glfwSetScrollCallback(eng.window, scrollCallBack);
}

void Update() 
{
	scene.Update(eng.WIDTH, eng.HEIGHT);
}

void Draw() 
{
	scene.engine.Clear();

	scene.Render();
}