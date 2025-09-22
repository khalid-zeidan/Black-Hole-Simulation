#pragma once
#include "allIncludes.h"

using namespace std;

class Engine
{
public:
	GLFWwindow* window;

	int WIDTH = 800;
	int HEIGHT = 600;
	float width = 100000000000.0f; // Width of the viewport in meters
	float height = 75000000000.0f; // Height of the viewport in meters

	Engine() {
		/* Initialize the library */
		if (!glfwInit())
			cerr << "Failed to initialize GLFW" << endl;

		window = glfwCreateWindow(WIDTH, HEIGHT, "Black Hole Simulation", NULL, NULL);
		
		if (!window)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwMakeContextCurrent(window);
		glViewport(0, 0, WIDTH, HEIGHT);
	}

	void Run() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		double left = -width;
		double right = width;
		double bottom = -height;
		double top = height;

		glOrtho(left, right, bottom, top, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
};

