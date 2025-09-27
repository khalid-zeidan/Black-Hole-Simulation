#pragma once
#include "allIncludes.h"
#include "BlackHole.h"
#include "Camera.h"
#include "Object.h"
#include "Engine.h"
#include "RayTracer.h"
#include "raytracer_source.h"

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
    FragColor = vec4(0.2, 0.2, 0.2, 0.5); // Gray color
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

GLuint CompileComputeShader(const char* computeSource) {
	// 1. Compile Compute Shader
	GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &computeSource, NULL);
	glCompileShader(compute);

	// Check compilation errors
	int success;
	char infoLog[512];
	glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(compute, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
		glDeleteShader(compute);
		return 0;
	}

	// 2. Link Program
	GLuint programID = glCreateProgram();
	glAttachShader(programID, compute);
	glLinkProgram(programID);

	// Check linking errors
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programID, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED (Compute)\n" << infoLog << std::endl;
		glDeleteShader(compute);
		glDeleteProgram(programID);
		return 0;
	}

	// 3. Clean up
	glDeleteShader(compute);

	return programID;
}

class Scene
{
public:
	Engine engine;
	RayTracer raytracer;

	BlackHole sagittariusA;
	Camera camera;

	// grid stuff
	GLuint gridShaderProgramID;
	GLuint gridVAO, gridVBO;
	int lineCount;

	GLuint quadShaderProgramID;
	vector<unsigned char> pixels;

	GLuint objectSSBO;
	int numObjects;

	// position, radius, color
	vector<Object> objects = {
	{vec3(-1e11, 5e10, 0), 1e10, vec3(255, 255, 255)},   // scaled down
	{vec3(4e10, 0, 0), 5e9, vec3(0, 255, 0)},
	{vec3(0.0f, 0.0f, 9e10), 1e9, vec3(255, 0, 0)}
	};

	Scene(vec3 pos, double mass) : sagittariusA(pos, mass) 
	{
		gridShaderProgramID = CompileShader(GRID_VERTEX_SHADER_SOURCE, GRID_FRAGMENT_SHADER_SOURCE);
		InitGrid();

		quadShaderProgramID = CompileShader(QUAD_VERTEX_SHADER_SOURCE, QUAD_FRAGMENT_SHADER_SOURCE);

		engine.computeProgram = CompileComputeShader(COMPUTE_SHADER_SOURCE);

		InitScreenTexture();
		InitSSBO();
    }

	void Update()
	{
		if (engine.computeProgram) 
		{
			// 1. Activate the Compute Shader
			glUseProgram(engine.computeProgram);

			// --- 2. SET ALL UNIFORMS ---

			// Black Hole / Raytracer Constants
			glUniform1f(glGetUniformLocation(engine.computeProgram, "u_RS"), (float)sagittariusA.R_S);
			glUniform1f(glGetUniformLocation(engine.computeProgram, "u_dLambda"), 1e8f);    // Example value: Step size (adjust as needed)
			glUniform1i(glGetUniformLocation(engine.computeProgram, "u_maxSteps"), 30000);   // Example value: Max iterations
			glUniform1f(glGetUniformLocation(engine.computeProgram, "u_escapeRadius"), 1e25f); // Example value
			glUniform1i(glGetUniformLocation(engine.computeProgram, "u_numObjects"), numObjects);

			// Camera Vectors
			vec3 camPos = camera.calculatePosition();
			vec3 camTarget = camera.target;

			glUniform3fv(glGetUniformLocation(engine.computeProgram, "u_cameraPos"), 1, glm::value_ptr(camPos));
			glUniform3fv(glGetUniformLocation(engine.computeProgram, "u_cameraTarget"), 1, glm::value_ptr(camTarget));

			// Screen Resolution
			glUniform1f(glGetUniformLocation(engine.computeProgram, "u_ScreenResolutionX"), (float)engine.WIDTH);
			glUniform1f(glGetUniformLocation(engine.computeProgram, "u_ScreenResolutionY"), (float)engine.HEIGHT);

			// --- 3. DISPATCH THE COMPUTE SHADER ---
			// Your GLSL is 16x16 local size. Calculate the number of work groups.
			const int local_size = 16;
			int num_groups_x = (engine.WIDTH + local_size - 1) / local_size;
			int num_groups_y = (engine.HEIGHT + local_size - 1) / local_size;

			glDispatchCompute((GLuint)num_groups_x, (GLuint)num_groups_y, 1);

			// --- 4. MEMORY BARRIER ---
			// Ensures the GPU finishes writing to the output image (binding 0)
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			// Clean up
			glUseProgram(0);
		}
		else 
		{
			cout << "FALLBACK ON CPU \n";

			// CPU fallback code
			for (int x = 0; x < engine.WIDTH; x++)
			{
				for (int y = 0; y < engine.HEIGHT; y++)
				{
					#pragma region camera-ray projection test
					/*// Build ray (we don’t actually use spherical components here)
					Ray ray = raytracer.GetInitialRay(camera, x, y, engine.WIDTH, engine.HEIGHT, sagittariusA);

					// --- Projection math (debug view) ---
					float ndcX = ((x + 0.5f) / (float)engine.WIDTH) * 2.0f - 1.0f;
					float ndcY = ((y + 0.5f) / (float)engine.HEIGHT) * 2.0f - 1.0f;

					float aspect = (float)engine.WIDTH / (float)engine.HEIGHT;
					float tanHalfFov = tanf(0.5f * radians(60.0f));

					float px = ndcX * aspect * tanHalfFov;
					float py = -ndcY * tanHalfFov;

					vec3 forward = normalize(camera.target - camera.calculatePosition());
					vec3 worldUp = vec3(0.0f, 1.0f, 0.0f);
					if (fabs(dot(forward, worldUp)) > 0.999f) {
						worldUp = vec3(0.0f, 0.0f, 1.0f);
					}
					vec3 right = normalize(cross(forward, worldUp));
					vec3 up = normalize(cross(right, forward));
					vec3 dir = normalize(forward + px * right + py * up);

					// --- Sphere intersection test ---
					vec3 hit;
					uint8_t r, g, b;
					if (intersectSphere(camera.calculatePosition(), dir, sagittariusA.position, sagittariusA.R_S, hit)) {
						float u = 0.5f + atan2(hit.y, hit.x) / (2.0f * M_PI);
						float v = 0.5f - asin(hit.z) / M_PI;

						int gridU = (int)(u * 10) % 2;
						int gridV = (int)(v * 10) % 2;
						bool checker = (gridU ^ gridV);

						if (checker) { r = 255; g = 255; b = 255; }
						else { r = 0;   g = 0;   b = 0; }
					}
					else {
						r = g = b = 50; // background gray
					}

					int index = (y * engine.WIDTH + x) * 3;
					pixels[index + 0] = r;
					pixels[index + 1] = g;
					pixels[index + 2] = b;*/
					#pragma endregion

					Ray ray = raytracer.GetInitialRay(camera, x, y, engine.WIDTH, engine.HEIGHT, sagittariusA);

					vec3 tracedColor = raytracer.TraceAndGetColor(ray, sagittariusA, objects);
					//vec3 tracedColor = vec3(150,100, 60);

					int index = (y * engine.WIDTH + x) * 3;
					pixels[index + 0] = tracedColor.x;// R
					pixels[index + 1] = tracedColor.y;// G
					pixels[index + 2] = tracedColor.z;// B
				}
			}

			glBindTexture(GL_TEXTURE_2D, engine.texture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, engine.WIDTH, engine.HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

private:

	void InitSSBO()
	{
		numObjects = objects.size();

		glGenBuffers(1, &objectSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectSSBO);

		glBufferData(GL_SHADER_STORAGE_BUFFER, objects.size() * sizeof(Object), objects.data(), GL_STATIC_DRAW);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, objectSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	// test function (skip this)
	bool intersectSphere(const vec3& origin, const vec3& dir, const vec3& center, float radius, vec3& hitPoint) {
		vec3 oc = origin - center;
		float a = dot(dir, dir);
		float b = 2.0 * dot(oc, dir);
		float c = dot(oc, oc) - radius * radius;
		
		float disc = b * b - 4.0 * a * c;
		if (disc < 0.0) return false;

		float t = (-b - sqrt(disc)) / (2.0 * a);
		if (t < 0.0) return false;

		hitPoint = origin + (float)t * dir;
		return true;
	}

	void InitScreenTexture() 
	{
		pixels.resize(engine.WIDTH * engine.HEIGHT * 3);

		if (engine.texture == 0) {
			glGenTextures(1, &engine.texture);
		}

		glBindTexture(GL_TEXTURE_2D, engine.texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, engine.WIDTH, engine.HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindImageTexture(0, engine.texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		glBindTexture(GL_TEXTURE_2D, 0);
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
		glDepthMask(GL_FALSE); // <--- CRITICAL: Prevents depth buffer writes
		glDisable(GL_DEPTH_TEST);

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
		//glDisable(GL_DEPTH_TEST);
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