#pragma once
#include "allIncludes.h"

using namespace std;
using namespace glm;

class Engine
{
private:
	// Checks a shader or program for compilation/linking errors
	void CheckShaderErrors(GLuint shader, const std::string& type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}

public:
	GLFWwindow* window;

	GLuint quadVAO, quadVBO, quadEBO;
	GLuint texture;

	GLuint computeProgram; // Already exists

	int   WIDTH = 800;
	int   HEIGHT = 600;
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
			cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
			throw std::runtime_error("Failed to initialize GLEW");
		}

		glViewport(0, 0, WIDTH, HEIGHT);
		SetupQuad();
		SetupTexture();
	}

	~Engine()
	{
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);
		glDeleteBuffers(1, &quadEBO);
		glDeleteTextures(1, &texture);
		glDeleteProgram(computeProgram);
		glfwTerminate();
	}

	void Clear()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Creates a shader program from a single compute shader source
	// returns 0 on failure
	GLuint CreateComputeProgram(const char* computeSource)
	{
		GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &computeSource, NULL);
		glCompileShader(compute);
		CheckShaderErrors(compute, "COMPUTE");

		GLuint program = glCreateProgram();
		glAttachShader(program, compute);
		glLinkProgram(program);
		CheckShaderErrors(program, "PROGRAM");

		glDeleteShader(compute);
		return program;
	}

	// Utility to create a generic program (used for grid and quad)
	GLuint CreateProgram(const char* vertexSource, const char* fragmentSource)
	{
		GLuint program = glCreateProgram();
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vertexSource, NULL);
		glCompileShader(vertex);
		CheckShaderErrors(vertex, "VERTEX");

		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragmentSource, NULL);
		glCompileShader(fragment);
		CheckShaderErrors(fragment, "FRAGMENT");

		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		glLinkProgram(program);
		CheckShaderErrors(program, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		return program;
	}


private:
	void SetupTexture()
	{
		glGenTextures(1, &texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Allocate memory for the texture (RGBA 32-bit float format for high precision)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void SetupQuad()
	{
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

		glBindVertexArray(0);
	}
};
