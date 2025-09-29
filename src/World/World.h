#pragma once
#include <unordered_map>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "Decoration.h"
#include "../Renderer/Shader.h"

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
	World();
	~World() = default;

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
		int SelectedVoxel = 1,
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

	glm::ivec3 ChunkToWorldCoords(const glm::ivec3& chunkCords, const glm::ivec3& chunkPos)
	{
		return glm::ivec3(
			(chunkPos.x * CHUNK_WIDTH) + chunkCords.x,
			(chunkPos.y * CHUNK_HEIGHT) + chunkCords.y,
			(chunkPos.z * CHUNK_DEPTH) + chunkCords.z
		);
	}

	void GenerateChunkDecorations(Chunk* chunk);

	void PlaceDecoration(glm::ivec3 Position,int DecorationID);

private:
	const glm::ivec3 m_ChunkSize = glm::ivec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH);
	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, ivec3Hash> m_Chunks;

	std::vector<Decoration> m_Decorations;
};