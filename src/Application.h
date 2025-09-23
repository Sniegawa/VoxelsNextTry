#pragma once

#include <cstdint>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Renderer/Shader.h"
#include "Renderer/Buffers.h"
#include "Renderer/Camera.h"

struct AppData
{
	uint32_t WindowWidth;
	uint32_t WindowHeight;
};

class App
{
public:
	App();
	~App();
	void Run();
private:
	GLFWwindow* m_Window;
	AppData m_data;

	Shader shader;
	VertexArray* VAO = nullptr;
	VertexBuffer* VBO = nullptr;
	ElementBuffer* EBO = nullptr;
	ComputeShader* computeShader = nullptr;

	Camera camera;
	uint32_t cameraUBO = 0;
};