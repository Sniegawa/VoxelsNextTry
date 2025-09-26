#pragma once

#include <filesystem>
#include <glm/glm.hpp>
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
public:
	ComputeShader(const std::filesystem::path& path, const std::filesystem::path& clearShaderPath, uint32_t width, uint32_t height);
	~ComputeShader();

	void CreateTexture(uint32_t width, uint32_t height);
	void Bind();
	void BindTextures();
	void ClearTextures();
	void Unbind();
	void Dispatch();

	uint32_t GetProgramID() const { return m_ProgramID; }
private:
	uint32_t m_ProgramID = 0;
	uint32_t m_ClearProgramID = 0;
	uint32_t m_ComputeTexture = 0;
	uint32_t m_DepthTexture = 0;

	uint32_t m_Width, m_Height;
};