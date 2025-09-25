#include "Application.h"
#include "PerlinNoise.hpp"

#include <cstdio>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>

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

	computeShader = new ComputeShader(std::filesystem::absolute("Shaders/computeExample.glsl"), m_data.WindowWidth, m_data.WindowHeight);

	glGenBuffers(1, &cameraUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);

	//const int GRID_SIZE = 512;

	const glm::ivec3 CHUNK_SIZE = glm::ivec3(32, 128, 32);

	std::vector<uint8_t> voxels(CHUNK_SIZE.x * CHUNK_SIZE.y * CHUNK_SIZE.z, 0);

	auto index = [&](int x, int y, int z) { return x + y * CHUNK_SIZE.x + z * CHUNK_SIZE.x * CHUNK_SIZE.y; };

	const siv::PerlinNoise::seed_type seed = 12456u;

	const siv::PerlinNoise perlin{ seed };

	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	if (0)
	{
		for (int x = 0; x < CHUNK_SIZE.x; ++x)
			for (int y = 0; y < CHUNK_SIZE.y; ++y)
				for (int z = 0; z < CHUNK_SIZE.z; ++z)
				{
					const double noise = perlin.octave3D_01((x * 0.01), (y * 0.01), (z * 0.01), 4);
					if (noise < 0.4f)
					{
						voxels[index(x, y, z)] = 1;
						voxelCount++;
					}
				}
	}
	else
	{
		for (int x = 0; x < CHUNK_SIZE.x; ++x)
		{
			for (int z = 0; z < CHUNK_SIZE.z; ++z)
			{
				const double noise = perlin.octave2D_01((x * 0.0015), (z * 0.0015), 8);
				int maxY = int(CHUNK_SIZE.y * noise);
				for (int y = 0; y < maxY; ++y)
				{	
					if (y >= maxY-5)
					{
						if(dist(rng) >= 0.5f)
							voxels[index(x, y, z)] = 5;
						else 
							voxels[index(x, y, z)] = 6;
					}
					else if (y > maxY - 200)
					{
						// Normalize x to [0,1]
						float chance = pow(float(maxY - y) / 200.0f, 2.6f);

						float r = dist(rng);      // random number 0..1
						voxels[index(x, y, z)] = (r < chance) ? (dist(rng) >= 0.02f ? 3 : 4) : (dist(rng) >= 0.5f ? 1 : 2);
					}
					else
					{
						if (dist(rng) >= 0.1f)
							voxels[index(x, y, z)] = 3;
						else
							voxels[index(x, y, z)] = 4;
					}
					voxelCount++;
				}
			}
		}
	}

	glGenTextures(1, &voxelTex3D);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, voxelTex3D);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, CHUNK_SIZE.x, CHUNK_SIZE.y, CHUNK_SIZE.z, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels.data());
	
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glUseProgram(computeShader->GetProgramID());

	glUniform1i(glGetUniformLocation(computeShader->GetProgramID(), "voxelTex"), 1);

	//glUniform1i(glGetUniformLocation(computeShader->GetProgramID(), "voxelGridSize"), GRID_SIZE);
	glUniform3i(glGetUniformLocation(computeShader->GetProgramID(), "ChunkSize"),CHUNK_SIZE.x, CHUNK_SIZE.y, CHUNK_SIZE.z);

	this->Run();
}

App::~App()
{
	delete VAO, VBO, EBO;
	delete computeShader;

	glDeleteTextures(1, &voxelTex3D);

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void App::Run()
{

	double lastTime = glfwGetTime();
	int frames = 0;
	while (!glfwWindowShouldClose(m_Window))
	{
		glClearColor(0.2f, 0.2f, 0.6f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		camera.CalculateMatricesFromInputs(m_Window);
		computeShader->Bind();
		glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
		CameraData& data = camera.getData();
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraData), &data);

		glUniform2i(glGetUniformLocation(computeShader->GetProgramID(),"iResolution"), m_data.WindowWidth, m_data.WindowHeight);

		computeShader->Dispatch();

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		computeShader->BindTexture();

		
		shader.Bind();
		VAO->Bind();

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		VAO->Unbind();

		glfwSwapBuffers(m_Window);
		glfwPollEvents();

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

