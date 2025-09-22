#include "Application.h"

#include <cstdio>

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

App::App()
{
	m_data.WindowWidth = 800;
	m_data.WindowHeight = 600;

	if (!glfwInit())
	{
		printf("Failed to initialize GLFW context");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	m_Window = glfwCreateWindow(m_data.WindowWidth, m_data.WindowHeight, "VOXELS",nullptr,nullptr);
	glfwMakeContextCurrent(m_Window);

	int version = gladLoadGL();
	if (version == 0)
	{
		printf("Failed to initialize OpenGL context\n");
		return;
	}
	glfwSetErrorCallback(error_callback);
	glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
	shader.Create(std::filesystem::absolute("Shaders/shader.vert"), std::filesystem::absolute("Shaders/shader.frag"));

	float quadVertices[] = {
		// positions         // texCoords
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // top-left
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f  // top-right
	};

	unsigned int quadIndices[] = {
		0, 1, 2,
		0, 2, 3
	};

	VAO = new VertexArray();
	VBO = new VertexBuffer(quadVertices, sizeof(quadVertices));
	EBO = new ElementBuffer(quadIndices, sizeof(quadIndices));
	VAO->Unbind();


	this->Run();
}

App::~App()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void App::Run()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glClearColor(0.2f, 0.2f, 0.6f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		
		shader.Bind();
		VAO->Bind();

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		VAO->Unbind();

		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}
}

