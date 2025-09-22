#pragma once

#include <filesystem>

class Shader
{
public:
	Shader();
	~Shader();

	void Create(const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path);
	void Bind();
	void Unbind();
private:
	uint32_t m_ProgramID = 0;
};


class ComputeShader
{
	ComputeShader();
	~ComputeShader();

	void Create(const std::string& source);
	void Bind();
	void Unbind();
	void Dispatch();
};