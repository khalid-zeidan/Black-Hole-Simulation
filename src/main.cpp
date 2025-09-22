#include "Engine.h"
#include "BlackHole.h"
#include "Ray.h"

using namespace glm;
using namespace std;

Engine engine;
BlackHole sagittariusA(vec2(engine.width/2, 0.0f), 8.54e36); // Mass of Sagittarius A* in kg
vector<Ray> rays;

int main(void)
{
	for (float y = -engine.height; y < engine.height; y += 1e10)
	{
		rays.push_back(Ray(vec2(-engine.width, y), vec2(1.0f, 0.0f)));
	}


	while (!glfwWindowShouldClose(engine.window))
	{
		engine.Run();
		sagittariusA.Draw();

		for (Ray& ray: rays)
		{
			ray.Draw();
			ray.Step();
		}

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}