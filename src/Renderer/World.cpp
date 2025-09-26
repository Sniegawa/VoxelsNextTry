#include "World.h"
#include <random>

bool World::IsSolid(glm::ivec3 worldPos) const
{
	glm::ivec3 chunkCoords = WorldToChunkCoords(worldPos);
	glm::ivec3 localCoords = WorldToLocalCoords(worldPos);

	auto it = m_Chunks.find(chunkCoords);
	if (it == m_Chunks.end()) return false;
	return it->second->isSolid(localCoords);
}


void World::SetVoxel(glm::ivec3 worldPos, uint8_t value)
{
	glm::ivec3 chunkCoords = WorldToChunkCoords(worldPos);
	glm::ivec3 localCoords = WorldToLocalCoords(worldPos);

	Chunk* chunk = GetOrCreateChunk(chunkCoords);
	chunk->SetVoxel(localCoords, value);
}

void World::ClearVoxel(glm::ivec3 worldPos)
{
	SetVoxel(worldPos, 0u);
}

bool World::HasChunk(const glm::ivec3& chunkPos) const {
	return m_Chunks.find(chunkPos) != m_Chunks.end();
}

Chunk* World::GetChunk(const glm::ivec3& chunkPos) {
	auto it = m_Chunks.find(chunkPos);
	if (it == m_Chunks.end()) return nullptr;
	return it->second.get();
}

// createChunk: inserts a new empty chunk (and marks dirty)
Chunk& World::CreateChunk(const glm::ivec3& chunkPos) {
	auto it = m_Chunks.find(chunkPos);
	if (it != m_Chunks.end()) return *it->second;

	auto chunk = std::make_unique<Chunk>(chunkPos);
	chunk->Create(); // alloc GPU texture etc.
	Chunk* ptr = chunk.get();
	m_Chunks.emplace(chunkPos, std::move(chunk));
	return *ptr;
}


Chunk* World::GetOrCreateChunk(const glm::ivec3& chunkCoords)
{
	Chunk* c = GetChunk(chunkCoords);
	if (c) return c;
	return &CreateChunk(chunkCoords);

}



bool World::raycastAndModify(
	glm::vec3 rayOrigin,
	glm::vec3 rayDir,
	float maxDistance,
	int radius,
	VoxelAction action,
	int maxSteps
)
{
	// Start voxel coordinates in world space (voxel units)
	glm::vec3 p = rayOrigin / VOXEL_SIZE;
	glm::ivec3 voxel = glm::floor(p);
	glm::ivec3 prev = voxel;

	// Step and distance to next boundary
	glm::ivec3 step;
	glm::vec3 tMax, tDelta;

	for (int i = 0; i < 3; ++i) {
		if (rayDir[i] > 0.0f) {
			step[i] = 1;
			float voxelBorder = voxel[i] + 1.0f;
			tMax[i] = (voxelBorder - p[i]) / rayDir[i] * VOXEL_SIZE;
			tDelta[i] = VOXEL_SIZE / rayDir[i];
		}
		else if (rayDir[i] < 0.0f) {
			step[i] = -1;
			float voxelBorder = voxel[i];
			tMax[i] = (voxelBorder - p[i]) / rayDir[i] * VOXEL_SIZE;
			tDelta[i] = -VOXEL_SIZE / rayDir[i];
		}
		else {
			step[i] = 0;
			tMax[i] = 1e30f;
			tDelta[i] = 1e30f;
		}
	}

	float traveled = 0.0f;

	for (int i = 0; i < maxSteps && traveled < maxDistance; ++i) {

		
		if (IsSolid(voxel)) {
			std::mt19937 rng(std::random_device{}());
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);
			// If radius > 0, apply modification to a sphere around hit
			if (radius > 1) {
				int rVoxels = radius;
				for (int x = -rVoxels; x <= rVoxels; ++x) {
					for (int y = -rVoxels; y <= rVoxels; ++y) {
						for (int z = -rVoxels; z <= rVoxels; ++z) {
							glm::ivec3 offset(x, y, z);
							glm::ivec3 target = voxel + offset;

							if (glm::length(glm::vec3(offset)) <= rVoxels) {
								if (action == VoxelAction::Remove)
									ClearVoxel(target);
								else if (action == VoxelAction::Add && !IsSolid(target))
								{
									if (dist(rng) >= 0.5f)
										SetVoxel(target, 1);
									else
										SetVoxel(target, 2);
									
								}
							}
						}
					}
				}
			}
			else {
				if (action == VoxelAction::Remove)
					ClearVoxel(voxel);
				else if (action == VoxelAction::Add)
				{
					if (dist(rng) >= 0.5f)
						SetVoxel(prev, 1);
					else
						SetVoxel(prev, 2);
				}
			}
			return true;
		}
		prev = voxel;
		// Walk
		if (tMax.x < tMax.y) {
			if (tMax.x < tMax.z) {
				voxel.x += step.x;
				traveled = tMax.x;
				tMax.x += tDelta.x;
			}
			else {
				voxel.z += step.z;
				traveled = tMax.z;
				tMax.z += tDelta.z;
			}
		}
		else {
			if (tMax.y < tMax.z) {
				voxel.y += step.y;
				traveled = tMax.y;
				tMax.y += tDelta.y;
			}
			else {
				voxel.z += step.z;
				traveled = tMax.z;
				tMax.z += tDelta.z;
			}
		}
		
	}

	return false; // no hit
}

uint64_t World::Render(ComputeShader* cs)
{
	uint64_t voxelcount = 0;
	for (auto& [chunkPos, chunkPtr] : m_Chunks) {
		if (!chunkPtr) continue;

		// Bind the chunk data to your compute shader
		chunkPtr->Bind(cs);
		voxelcount += chunkPtr->getDebugVoxelCount();
		// Dispatch your compute shader for this chunk
		cs->Dispatch();
	}

	return voxelcount;
}