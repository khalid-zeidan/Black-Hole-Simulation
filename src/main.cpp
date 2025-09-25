#include "allIncludes.h"
#include "Scene.h"

using namespace glm;
using namespace std;

Engine& eng = scene.engine;

#pragma region function declaration
void start();
#pragma endregion

int main()
{
	start();

	while (!glfwWindowShouldClose(eng.window))
	{
		scene.Update();
		scene.Render();

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