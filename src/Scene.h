#pragma once
#include "allIncludes.h"
#include "BlackHole.h"
#include "Camera.h"
#include "Object.h"
#include "Engine.h"
#include "RayTracer.h"

using namespace std;
using namespace glm;

#pragma region shader definitions
#pragma region grid
const char* GRID_VERTEX_SHADER_SOURCE = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)glsl";

const char* GRID_FRAGMENT_SHADER_SOURCE = R"glsl(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.5, 0.5, 0.5, 1.0); // Gray color
}
)glsl";
#pragma endregion

#pragma region quad
const char* QUAD_VERTEX_SHADER_SOURCE = R"glsl(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)glsl";

const char* QUAD_FRAGMENT_SHADER_SOURCE = R"glsl(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D screenTexture;

void main()
{
    FragColor = texture(screenTexture, TexCoord);
}
)glsl";
#pragma endregion
#pragma endregion

GLuint CompileShader(const char* vertexSource, const char* fragmentSource)  {
	// 1. Compile Vertex Shader
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSource, NULL);
	glCompileShader(vertex);

	// 2. Compile Fragment Shader
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, NULL);
	glCompileShader(fragment);

	// 3. Link Program
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertex);
	glAttachShader(programID, fragment);
	glLinkProgram(programID);

	// Minimal Error Checking (Optional but recommended)
	int success;
	char infoLog[512];
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programID, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// 4. Clean up (shaders are linked into the program, so we can delete them)
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return programID;
}

class Scene
{
public:
	Engine engine;
	RayTracer raytracer;

	BlackHole sagittariusA;
	Camera camera;

	vector<Object> objects;

	// grid stuff
	GLuint gridShaderProgramID;
	GLuint gridVAO, gridVBO;
	int lineCount;

	GLuint quadShaderProgramID;
	vector<unsigned char> pixels;

	// position, color, radius
	//vector<Object> objectsData = {
	//	{vec3(4e11f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), 4e5f},
	//	{vec3(0.0f, 4e11f, 4e11f), vec4(0.4f, 0.2f, 1.0f, 1.0f), 4e5f}
	//}; 

	Scene(vec3 pos, double mass) : sagittariusA(pos, mass) 
	{
		gridShaderProgramID = CompileShader(GRID_VERTEX_SHADER_SOURCE, GRID_FRAGMENT_SHADER_SOURCE);
		InitGrid();

		quadShaderProgramID = CompileShader(QUAD_VERTEX_SHADER_SOURCE, QUAD_FRAGMENT_SHADER_SOURCE);

		InitScreenTexture();
    }

	void Update()
	{
		for (int x = 0; x < engine.WIDTH; x++)
		{
			for (int y = 0; y < engine.HEIGHT; y++)
			{
				Ray ray = raytracer.GetInitialRay(camera, x, y, engine.WIDTH, engine.HEIGHT);

				vec3 tracedColor = raytracer.TraceAndGetColor(ray, sagittariusA, objects);

				int index = (y * engine.WIDTH + x) * 3;
				pixels[index + 0] = tracedColor.x;// R
				pixels[index + 1] = tracedColor.y;// G
				pixels[index + 2] = tracedColor.z;// B
			}
		}

		//for (size_t i = 0; i < engine.WIDTH * engine.HEIGHT / 2; i++)
		//{
		//	pixels[i * 3 + 0] = 255;// R
		//	pixels[i * 3 + 1] = 255;	// G
		//	pixels[i * 3 + 2] = 255;	// B
		//}

		glBindTexture(GL_TEXTURE_2D, engine.texture);
		// Use GL_BGR for faster upload if data is stored as R,G,B (though GL_RGB is fine too)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, engine.WIDTH, engine.HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	void InitScreenTexture() 
	{
		pixels.resize(engine.WIDTH * engine.HEIGHT * 3); // RGB per pixel

		// Bind and allocate memory for the texture
		glBindTexture(GL_TEXTURE_2D, engine.texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, engine.WIDTH, engine.HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void InitGrid() {
		// Use a grid size that is manageable for the current camera zoom
		float size = 1e12f;
		float step = 2e10f;
		std::vector<glm::vec3> vertices;

		for (float i = -size; i <= size; i += step) {
			// X-axis lines
			vertices.push_back(glm::vec3(i, 0.0f, -size));
			vertices.push_back(glm::vec3(i, 0.0f, size));

			// Z-axis lines
			vertices.push_back(glm::vec3(-size, 0.0f, i));
			vertices.push_back(glm::vec3(size, 0.0f, i));
		}
		lineCount = vertices.size();

		// 1. Generate and bind VAO/VBO
		glGenVertexArrays(1, &gridVAO);
		glGenBuffers(1, &gridVBO);
		glBindVertexArray(gridVAO);

		// 2. Upload vertex data
		glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		// 3. Configure Vertex Attributes (location 0 in shader)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);

		// 4. Unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void DrawQuad() const
	{
		glDisable(GL_DEPTH_TEST); // The quad is always on top

		glUseProgram(quadShaderProgramID);

		// Bind the texture to Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, engine.texture);
		glUniform1i(glGetUniformLocation(quadShaderProgramID, "screenTexture"), 0);

		// Draw the quad
		glBindVertexArray(engine.quadVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(0);
		glEnable(GL_DEPTH_TEST);
	}

	void DrawGrid() const {
		mat4 projection = camera.GetProjectionMatrix((float)engine.WIDTH, (float)engine.HEIGHT);
		mat4 view = camera.GetViewMatrix();
		mat4 model = mat4(1.0f);

		// 1. Use Shader and Set Uniforms
		glEnable(GL_DEPTH_TEST);

		glUseProgram(gridShaderProgramID);

		// Find Uniform Locations and set them
		glUniformMatrix4fv(glGetUniformLocation(gridShaderProgramID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(gridShaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(gridShaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// 2. Draw Grid using VAO
		glLineWidth(1.0f);
		glBindVertexArray(gridVAO);
		glDrawArrays(GL_LINES, 0, lineCount);
		glBindVertexArray(0);
		glDisable(GL_DEPTH_TEST);
	}

public:
	void Render()
	{
		engine.Clear();

		DrawQuad();
		DrawGrid();
	}

}scene(vec3(0.0f, 0.0f, 0.0f), 8.54e36); //pos of blackhole, mass of sagittarius A*

#pragma region Camera Input detection
//checks if mouse buttons are pressed
void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		scene.camera.dragging = true;
	}
	else 
	{
		scene.camera.dragging = false;
		scene.camera.firstMouse = true;
	}
}

// checks mouse move positions
void mouseMoveCallBack(GLFWwindow* window, double xpos, double ypos)
{
	if (scene.camera.firstMouse) 
	{
		scene.camera.lastMouseX = xpos;
		scene.camera.lastMouseY = ypos;
		scene.camera.firstMouse = false;
	}

	float deltaX = (float)(xpos - scene.camera.lastMouseX);
	float deltaY = (float)(scene.camera.lastMouseY - ypos);

	scene.camera.lastMouseX = xpos;
	scene.camera.lastMouseY = ypos;

	if (scene.camera.dragging) {
		// Pass the delta values to the camera's rotate method
		scene.camera.Rotate(deltaX, deltaY);
	}
}

//checks mouse scroll 
void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
	scene.camera.Zoom(yoffset);
}
#pragma endregion