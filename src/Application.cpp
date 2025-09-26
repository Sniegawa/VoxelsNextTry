#include "Application.h"
 

#include <cstdio>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>



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
static uint64_t voxelCount = 0;

App::App()
{
	{
		m_data.WindowWidth = 1600;
		m_data.WindowHeight = 900;

		if (!glfwInit())
		{
			printf("Failed to initialize GLFW context");
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_Window = glfwCreateWindow(m_data.WindowWidth, m_data.WindowHeight, "VOXELS", nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);

		int version = gladLoadGL();
		if (version == 0)
		{
			printf("Failed to initialize OpenGL context\n");
			return;
		}
		glfwSetErrorCallback(error_callback);
		glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
	}

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 460");


	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	camera.Setup(m_Window);

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

	computeShader = new ComputeShader(std::filesystem::absolute("Shaders/computeVoxelTraversal.glsl"), std::filesystem::absolute("Shaders/computeClearTextures.glsl"), m_data.WindowWidth, m_data.WindowHeight);

	glGenBuffers(1, &cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);

	const int ChunksRowSize = 10;
	const int ChunksColumnSize = 10;
	Chunks.resize(ChunksRowSize * ChunksColumnSize);
	for (int x = 0; x < ChunksRowSize; ++x)
	{
		for (int z = 0; z < ChunksColumnSize; ++z)
		{
			Chunks[x + z * ChunksRowSize] = std::make_unique<Chunk>(glm::ivec3(x, 0, z));
			Chunks[x + z * ChunksRowSize]->Create();
			voxelCount += Chunks[x + z * ChunksRowSize]->getDebugVoxelCount();
		}
	}

	this->Run();
}

App::~App()
{
	delete VAO, VBO, EBO;
	delete computeShader;

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void App::Run()
{

	double lastTime = glfwGetTime();
	float lastDT = glfwGetTime();
	int frames = 0;
	while (!glfwWindowShouldClose(m_Window))
	{
		glClearColor(0.2f, 0.2f, 0.6f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		computeShader->ClearTextures();
		float currentDT = glfwGetTime();
		float dt = lastDT - currentDT;
		lastDT = currentDT;
		camera.CalculateMatricesFromInputs(m_Window,dt);
		computeShader->Bind();
		computeShader->BindTextures();

		glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
		CameraData& data = camera.getData();
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraData), &data);

		glUniform2i(glGetUniformLocation(computeShader->GetProgramID(),"iResolution"), m_data.WindowWidth, m_data.WindowHeight);

		for (const auto& chunk : Chunks)
		{
			chunk->Bind(computeShader);
			computeShader->Dispatch();
			
		}

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		
		shader.Bind();
		VAO->Bind();

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		VAO->Unbind();

		glfwPollEvents();

		/*ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Simple window
		ImGui::Begin("Hello, ImGui!");

		

		ImGui::End();

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		*/
		glfwSwapBuffers(m_Window);
		double currentTime = glfwGetTime();
		frames++;
		if (currentTime - lastTime >= 1.0) { // every second
			double fps = double(frames) / (currentTime - lastTime);
			double ms = 1000.0 / fps;

			std::ostringstream title;
			title << "FPS: " << std::fixed << std::setprecision(1) << fps
				<< " | " << std::setprecision(6) << ms << " ms" << " Voxels : " << voxelCount;
			glfwSetWindowTitle(m_Window, title.str().c_str());

			frames = 0;
			lastTime = currentTime;
		}

		
	}
}

