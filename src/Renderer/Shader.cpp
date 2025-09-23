#include "Shader.h"

#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

std::string LoadShaderFromFile(const std::filesystem::path& path)
{
	std::ifstream file(path.string());

	if (!file.is_open())
	{
		std::cout << "Cannot open file: " << path.string() << std::endl;
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

Shader::Shader()
{ 
	
}

Shader::~Shader()
{
	glDeleteProgram(m_ProgramID);
}

void Shader::Create(const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path)
{
	std::string vertex_content = LoadShaderFromFile(vertex_path);
	std::string fragment_content = LoadShaderFromFile(fragment_path);

	const char* vertex_source = vertex_content.c_str();
	const char* fragment_source = fragment_content.c_str();

	m_ProgramID = glCreateProgram();

	uint32_t vertex, fragment;

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertex_source, NULL);
	glCompileShader(vertex);
	{
		int  success;
		char infoLog[512];
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR VERTEX COMPILATION FAILED\n" << infoLog << std::endl;
		}
	}
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragment_source, NULL);
	glCompileShader(fragment);
	{
		int  success;
		char infoLog[512];
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR FRAGMENT COMPILATION FAILED\n" << infoLog << std::endl;
		}
	}

	glAttachShader(m_ProgramID, vertex);
	glAttachShader(m_ProgramID, fragment);
	glLinkProgram(m_ProgramID);
	{
		int  success;
		char infoLog[512];
		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(m_ProgramID, 512, NULL, infoLog);
			std::cout << "ERROR PROGRAM LINKING FAILED \n" << infoLog << std::endl;
		}
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}



void Shader::Bind()
{
	glUseProgram(m_ProgramID);
}

void Shader::Unbind()
{
	glUseProgram(0);
}

ComputeShader::ComputeShader(const std::filesystem::path& path,uint32_t width, uint32_t height)
	: m_Width(width),m_Height(height)
{
	std::string content = LoadShaderFromFile(path);

	const char* content_source = content.c_str();

	uint32_t computeID = glCreateShader(GL_COMPUTE_SHADER);

	glShaderSource(computeID, 1, &content_source, nullptr);
	glCompileShader(computeID);

	GLint success;
	glGetShaderiv(computeID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[1024];
		glGetShaderInfoLog(computeID, 1024, nullptr, infoLog);
		std::cerr << "Compute Shader Compilation Error:\n" << infoLog << "\n";
	}

	m_ProgramID = glCreateProgram();
	glAttachShader(m_ProgramID, computeID);
	glLinkProgram(m_ProgramID);

	glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[1024];
		glGetProgramInfoLog(m_ProgramID, 1024, nullptr, infoLog);
		std::cerr << "Compute Shader Linking Error:\n" << infoLog << "\n";
	}

	glDeleteShader(computeID);

	CreateTexture(width, height);
}

ComputeShader::~ComputeShader()
{
	glDeleteProgram(m_ProgramID);
	glDeleteTextures(1,&m_ComputeTexture);
}

void ComputeShader::CreateTexture(uint32_t width, uint32_t height)
{
	glGenTextures(1, &m_ComputeTexture);
	glBindTexture(GL_TEXTURE_2D, m_ComputeTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glBindImageTexture(0, m_ComputeTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

}

void ComputeShader::Dispatch()
{
	glUseProgram(m_ProgramID);

	GLuint groupX = (GLuint)ceil(m_Width / 16.0f);
	GLuint groupY = (GLuint)ceil(m_Height / 16.0f);

	glDispatchCompute(groupX, groupY, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void ComputeShader::BindTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ComputeTexture);
}

void ComputeShader::Bind()
{
	glUseProgram(m_ProgramID);
}

void ComputeShader::Unbind()
{
	glUseProgram(0);
}