#include "Engine.h"
#include "BlackHole.h"
#include "Ray.h"
#include "Physics.h"

using namespace glm;
using namespace std;

Engine engine;
BlackHole sagittariusA(vec2(0.0f, 0.0f), 8.54e36); // Mass of Sagittarius A* in kg
vector<Ray> rays;

int main(void)
{
	for (float y = -engine.height*3; y < engine.height*3; y += 1e10)
	{
		rays.push_back(Ray(vec2(-engine.width, y), vec2(1e8, 0.75e8)));
	}


	while (!glfwWindowShouldClose(engine.window))
	{
		engine.Run();
		sagittariusA.Draw();

		for (auto& ray: rays)
		{
			RK4Step(ray, sagittariusA.eventHorizonRadius, 1);
			ray.Step(); 
			ray.Draw();
		}

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}