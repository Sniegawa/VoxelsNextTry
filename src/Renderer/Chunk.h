#pragma once

#include <stdint.h>

#include <glm/glm.hpp>
#include <array>

#include "Shader.h"

#define CHUNK_WIDTH 128
#define CHUNK_HEIGHT 256
#define CHUNK_DEPTH 128
#define VOXEL_SIZE 0.0625f

class Chunk
{
public:
	Chunk(glm::ivec3 pos = glm::ivec3(0));
	~Chunk();

	void Create();
	void Bind(ComputeShader* cs);

	inline uint8_t& At(int x, int y, int z) {
		return m_Data[x + CHUNK_WIDTH * (z + CHUNK_DEPTH * y)];
	}
	inline const uint8_t& At(int x, int y, int z) const {
		return m_Data[x + CHUNK_WIDTH * (z + CHUNK_DEPTH * y)];
	}

	void SetVoxel(int x, int y, int z, uint8_t value) {
		At(x, y, z) = value;
		m_Dirty = true;
	}

	inline uint32_t getDebugVoxelCount() { return voxelsCount; }

private:
	std::array<uint8_t, CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH> m_Data;
	uint32_t m_TextureID;
	glm::ivec3 m_ChunkPosition;
	bool m_Dirty = false; //Whether texture needs updating

	//DebugData
	uint32_t voxelsCount;
};