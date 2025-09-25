#pragma once
#include "allIncludes.h"

using namespace std;
using namespace glm;

class Engine
{
public:
	GLFWwindow* window;

	GLuint quadVAO;
	GLuint texture; 

	int WIDTH = 800;
	int HEIGHT = 600;
	float width = 100000000000.0f; // Width of the viewport in meters
	float height = 75000000000.0f; // Height of the viewport in meters

	Engine() 
	{
		/* Initialize the library */
		if (!glfwInit())
			cerr << "Failed to initialize GLFW" << endl;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Black Hole Simulation", NULL, NULL);
		
		if (!window)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwMakeContextCurrent(window);

		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << endl;
			glfwTerminate();
			throw std::runtime_error("GLEW initialization failed");
		}

		// Optional: Check the OpenGL version actually running
		cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;

		glViewport(0, 0, WIDTH, HEIGHT);
	}

	void Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
};

