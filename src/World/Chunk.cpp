#include "Chunk.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "../PerlinNoise.hpp"
#include <random>

Chunk::Chunk(glm::ivec3 pos)
	: m_ChunkPosition(pos)
{
	
}

Chunk::~Chunk()
{
	glDeleteTextures(1, &m_TextureID);
}

void Chunk::CreateTexture()
{
	glDeleteTextures(1, &m_TextureID);

	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_3D, m_TextureID);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, m_Data.data());

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glBindTexture(GL_TEXTURE_3D, 0);

	m_Dirty = false;
}

void Chunk::Create()
{
	m_Data.fill(0u);


	auto index = [&](int x, int y, int z) { return x + y * CHUNK_WIDTH + z * CHUNK_WIDTH * CHUNK_HEIGHT; };

	const siv::PerlinNoise::seed_type seed = 12456u;

	const siv::PerlinNoise perlin{ seed };

	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);


	for (int x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (int z = 0; z < CHUNK_DEPTH; ++z)
		{
			const float noise_scale = 0.001;
			const double noise = perlin.octave2D_01(( (x+(m_ChunkPosition.x*CHUNK_WIDTH)) * noise_scale), ((z + (m_ChunkPosition.z * CHUNK_DEPTH)) * noise_scale), 8);
			int maxY = int(CHUNK_HEIGHT * noise);
			m_Heightmap[x + CHUNK_WIDTH * z] = maxY;
			for (int y = 0; y < maxY; ++y)
			{
				if (y >= maxY - 5)
				{
					if (dist(rng) >= 0.5f)
						m_Data[index(x, y, z)] = 5;
					else
						m_Data[index(x, y, z)] = 6;
				}
				else if (y > maxY - 200)
				{
					float chance = pow(float(maxY - y) / 200.0f, 2.6f);

					float r = dist(rng);
					m_Data[index(x, y, z)] = (r < chance) ? (dist(rng) >= 0.02f ? 3 : 4) : (dist(rng) >= 0.5f ? 1 : 2);
				}
				else
				{
					if (dist(rng) >= 0.1f)
						m_Data[index(x, y, z)] = 3;
					else
						m_Data[index(x, y, z)] = 4;
				}
				voxelsCount++;
			}
		}
	}

	CreateTexture();
}

void Chunk::Bind(ComputeShader* cs)
{
	if (m_Dirty)
		CreateTexture();
	//glUseProgram(cs->GetProgramID());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, m_TextureID);

	glUniform1i(glGetUniformLocation(cs->GetProgramID(), "voxelTex"), 2);

	glm::vec3 GridPosition = (glm::vec3)m_ChunkPosition * glm::vec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH) * VOXEL_SIZE;
	glUniform3fv(glGetUniformLocation(cs->GetProgramID(), "ChunkPosition"), 1, glm::value_ptr(GridPosition));
	glUniform3i(glGetUniformLocation(cs->GetProgramID(), "ChunkSize"), CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH);

	glUniform1f(glGetUniformLocation(cs->GetProgramID(), "voxelSize"), VOXEL_SIZE);
}
