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
		rays.push_back(Ray(vec2(-engine.width, y), vec2(1.0f, 0.0f)));
	}


	while (!glfwWindowShouldClose(engine.window))
	{
		engine.Run();
		sagittariusA.Draw();

		for (auto& ray: rays)
		{
			GeoDesic(ray, sagittariusA.eventHorizonRadius);
			ray.Draw();
			ray.Step(sagittariusA.eventHorizonRadius, 3); //1e-1
		}

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}