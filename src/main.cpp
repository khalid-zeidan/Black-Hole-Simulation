#include "Engine.h"
#include "BlackHole.h"
#include "Ray.h"
#include "Physics.h"
#include "Scene.h"

using namespace glm;
using namespace std;

Engine engine;
Scene scene(vec3(0.0f, 0.0f, 0.0f), 8.54e36); //pos of blackhole, mass of sagittarius A*

int main(void)
{
	while (!glfwWindowShouldClose(engine.window))
	{
		engine.Run();
		scene.Render();

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}