#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Decoration
{
	Decoration(int width, int height, int depth)
		: Size(glm::ivec3(width,height,depth))
	{
		Data.resize(width * height * depth);
	}
	Decoration(glm::ivec3 size)
		: Size(size)
	{
		Data.resize(size.x * size.y * size.z);
	}

	Decoration(std::vector<uint8_t>& data, glm::ivec3 size)
		: Data(data), Size(size) 
	{
		assert(size.x * size.y * size.z == data.size());
	};

	~Decoration() = default;

	glm::ivec3 Size;

	std::vector<uint8_t> Data;
};