#pragma once
#include <unordered_map>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "Shader.h"


struct ivec3Hash {
	std::size_t operator()(const glm::ivec3& v) const noexcept {
		return (std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1)) ^ (std::hash<int>()(v.z) << 2);
	}
};

enum class VoxelAction {
	Add,
	Remove
};

class World {
public:
	World() {}

	bool IsSolid(glm::ivec3 worldPos) const;
	void SetVoxel(glm::ivec3 worldPos, uint8_t value = 1);
	void ClearVoxel(glm::ivec3 worldPos);

	bool HasChunk(const glm::ivec3& chunkPos) const;
	Chunk* GetChunk(const glm::ivec3& chunkPos);                // nullptr if missing
	Chunk& CreateChunk(const glm::ivec3& chunkPos);
	Chunk* GetOrCreateChunk(const glm::ivec3& chunkCoords);

	bool raycastAndModify(
		glm::vec3 rayOrigin,
		glm::vec3 rayDir,
		float maxDistance,
		int radius,
		VoxelAction action,
		int maxSteps = 512
	);


	uint64_t Render(ComputeShader* cs);

	glm::ivec3 getChunkSize() const { return m_ChunkSize; }

private:
	glm::ivec3 WorldToChunkCoords(const glm::ivec3& worldPos) const {
		return glm::ivec3(
			floor((float)worldPos.x / CHUNK_WIDTH),
			floor((float)worldPos.y / CHUNK_HEIGHT),
			floor((float)worldPos.z / CHUNK_DEPTH)
		);
	}

	// Map world voxel position -> chunk-local coords
	glm::ivec3 WorldToLocalCoords(const glm::ivec3& worldPos) const {
		return glm::ivec3(
			(worldPos.x % CHUNK_WIDTH + CHUNK_WIDTH) % CHUNK_WIDTH,
			(worldPos.y % CHUNK_HEIGHT + CHUNK_HEIGHT) % CHUNK_HEIGHT,
			(worldPos.z % CHUNK_DEPTH + CHUNK_DEPTH) % CHUNK_DEPTH
		);
	}
private:
	const glm::ivec3 m_ChunkSize = glm::ivec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH);
	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, ivec3Hash> m_Chunks;
};