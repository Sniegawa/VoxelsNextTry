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

ComputeShader::ComputeShader()
{

}

ComputeShader::~ComputeShader()
{

}

void ComputeShader::Create(const std::string& source)
{

}

void ComputeShader::Dispatch()
{

}
