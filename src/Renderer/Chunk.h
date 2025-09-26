#pragma once

#include <stdint.h>

#include <glm/glm.hpp>
#include <array>

#include "Shader.h"

#define CHUNK_WIDTH 128
#define CHUNK_HEIGHT 512
#define CHUNK_DEPTH 128
#define VOXEL_SIZE 0.125f

class Chunk
{
public:
	Chunk(glm::ivec3 pos = glm::ivec3(0));
	~Chunk();

	void Create();
	void CreateTexture();
	void Bind(ComputeShader* cs);

	bool isSolid(glm::ivec3 localPos) const 
	{
		if (!inBounds(localPos)) return false;
		return m_Data[index(localPos)] != 0;
	}

	inline uint8_t& At(int x, int y, int z) 
	{
		return m_Data[index(glm::ivec3(x,y,z))];
	}

	inline const uint8_t& At(int x, int y, int z) const 
	{
		return m_Data[index(glm::ivec3(x,y,z))];
	}

	void SetVoxel(int x, int y, int z, uint8_t value) 
	{
		At(x, y, z) = value;
		m_Dirty = true;

		if (value == 0)
			voxelsCount--;
		else
			voxelsCount++;
	}

	void SetVoxel(glm::ivec3 pos, uint8_t value)
	{
		At(pos.x,pos.y,pos.z) = value;
		m_Dirty = true;

		if (value == 0)
			voxelsCount--;
		else
			voxelsCount++;
	}

	inline uint32_t getDebugVoxelCount() { return voxelsCount; }

private:
	bool inBounds(glm::ivec3 p) const {
		return (p.x >= 0 && p.y >= 0 && p.z >= 0 &&
			p.x < CHUNK_WIDTH && p.y < CHUNK_HEIGHT && p.z < CHUNK_DEPTH);
	}

	int index(glm::ivec3 p) const {
		return p.x + CHUNK_WIDTH * (p.y + CHUNK_HEIGHT * p.z);
	}

private:
	std::array<uint8_t, CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH> m_Data;
	uint32_t m_TextureID;
	glm::ivec3 m_ChunkPosition;
	bool m_Dirty = false; //Whether texture needs updating

	//DebugData
	uint32_t voxelsCount;
};