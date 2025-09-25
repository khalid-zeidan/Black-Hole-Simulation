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

	GLuint quadVBO, quadEBO;

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

		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f, // top left
			-1.0f, -1.0f,  0.0f, 0.0f, // bottom left
			 1.0f, -1.0f,  1.0f, 0.0f, // bottom right
			 1.0f,  1.0f,  1.0f, 1.0f  // top right
		};

		unsigned int quadIndices[] = {
			0, 1, 2, // first triangle
			0, 2, 3  // second triangle
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glGenBuffers(1, &quadEBO);

		glBindVertexArray(quadVAO);

		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// texture coord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	void UpdateTextureFormatForCompute()
	{
		// Re-create the texture with GL_RGBA8 to allow it to be bound as a compute image
		if (texture != 0) glDeleteTextures(1, &texture);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Use GL_RGBA8 for the internal format, matching the GLSL layout(rgba8, binding=0)
		// Set the output format to GL_RGBA, as the compute shader writes 4 components (RGBA)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		// Standard texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	~Engine() {
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);
		glDeleteBuffers(1, &quadEBO);
		glDeleteTextures(1, &texture);
	}
};

