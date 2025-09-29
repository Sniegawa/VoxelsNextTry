#include "World.h"

#include <random>
#include "../PerlinNoise.hpp"

World::World()
{
	std::vector<uint8_t> tree = {
		// z = 0
		// y = 
		0,0,0,   // y=0, x=0..2
		0,0,0,   // y=1
		0,0,0,   // y=2
		0,5,0,   // y=5
		0,0,0,   // y=4

		// z = 1
		0,1,0,   // y=0
		0,1,0,   // y=1
		0,1,0,   // y=2
		5,5,5,   // y=5
		0,5,0,    // y=4

		// z = 1
		0,0,0,   // y=0
		0,0,0,   // y=1
		0,0,0,   // y=2
		0,5,0,   // y=5
		0,0,0    // y=4
	};
	Decoration tree_dec = Decoration(tree, glm::ivec3(3, 5, 3));
	m_Decorations.push_back(tree_dec);

	constexpr int TREE_WIDTH = 5;
	constexpr int TREE_DEPTH = 5;
	constexpr int TREE_HEIGHT = 10;

	std::vector<uint8_t> tree2(TREE_WIDTH * TREE_HEIGHT * TREE_DEPTH, 0); // all empty

	// Lambda to compute 1D index
	auto idx = [&](int x, int y, int z) { return x + y * TREE_WIDTH + z * TREE_WIDTH * TREE_HEIGHT; };

	// Trunk (1󪻕) in the center bottom
	int centerX = TREE_WIDTH / 2;
	int centerZ = TREE_DEPTH / 2;
	for (int y = 0; y < 5; ++y) {
		tree2[idx(centerX, y, centerZ)] = 1; // bark
	}

	// Leaves (cube canopy) from y=5 to y=9
	for (int y = 5; y < TREE_HEIGHT; ++y) {
		for (int z = 1; z < TREE_DEPTH - 1; ++z) {
			for (int x = 1; x < TREE_WIDTH - 1; ++x) {
				tree2[idx(x, y, z)] = 5; // leaves
			}
		}
	}

	// Create decoration
	Decoration tree_dec2 = Decoration(tree2, glm::ivec3(TREE_WIDTH, TREE_HEIGHT, TREE_DEPTH));
	m_Decorations.push_back(tree_dec2);
}

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
	if (worldPos.y >= CHUNK_HEIGHT || worldPos.y < 0)
		return;
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
	printf("Generating chunk number : %i position %i,%i,%i\n", m_Chunks.size(),chunkPos.x,chunkPos.y,chunkPos.z);
	GenerateChunkDecorations(GetChunk(chunkPos));
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
	int SelectedVoxel,
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
										SetVoxel(target, SelectedVoxel);
									else
										SetVoxel(target, SelectedVoxel + 1);
									
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
						SetVoxel(prev, SelectedVoxel);
					else
						SetVoxel(prev, SelectedVoxel + 1);
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

void World::GenerateChunkDecorations(Chunk* chunk)
{
	const siv::PerlinNoise::seed_type seed = 2456u;

	const siv::PerlinNoise perlin{ seed };

	for (int x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (int z = 0; z < CHUNK_DEPTH; ++z)
		{
			int worldx = x + (chunk->m_ChunkPosition.x * CHUNK_WIDTH);
			int worldz = z + (chunk->m_ChunkPosition.z * CHUNK_DEPTH);
			float noise = perlin.octave2D_01(worldx, worldz, 4);
			//Treshold tbd
			if (noise >= 0.85f)
			{
				std::mt19937 rng(std::random_device{}());
				std::uniform_real_distribution<float> dist(0.0f, 1.0f);
				int MaxY = chunk->m_Heightmap[x + CHUNK_WIDTH * z];
				glm::ivec3 DecorationBaseCoords = ChunkToWorldCoords(glm::ivec3(x, MaxY + 1, z), chunk->m_ChunkPosition);
				PlaceDecoration(DecorationBaseCoords, (dist(rng) >= 0.5f ? 0 : 1));
			}
		}
	}
}

bool FitsInChunk(const glm::ivec3& pos, const glm::ivec3& decSize)
{
	if (pos.x < 0 || pos.y < 0 || pos.z < 0) return false;
	if (pos.x + decSize.x > CHUNK_WIDTH)  return false;
	if (pos.y + decSize.y > CHUNK_HEIGHT) return false;
	if (pos.z + decSize.z > CHUNK_DEPTH) return false;
	return true;
}


void World::PlaceDecoration(glm::ivec3 Position, int DecorationID)
{
	const Decoration& dec = m_Decorations[DecorationID];

	const glm::ivec3& decSize = dec.Size;

	glm::ivec3 chunkPos = WorldToChunkCoords(Position + decSize);

	if (m_Chunks.find(chunkPos) == m_Chunks.end())
		return;



	for (int x = 0; x < decSize.x; ++x)
	{
		for (int y = 0; y < decSize.y; ++y)
		{
			for (int z = 0; z < decSize.z; ++z)
			{
				if (dec.Data[x + y * decSize.x + z * decSize.x * decSize.y])
					SetVoxel(Position + glm::ivec3(x, y, z), dec.Data[x + y * decSize.x + z * decSize.x * decSize.y]);
			}
		}
	}
}