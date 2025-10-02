#pragma once

#include <cstdint>
#include <cstddef>

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void Bind();
	void Unbind();

private:
	uint32_t m_ID;
};

class VertexBuffer
{
public:
	VertexBuffer(float* vertices, size_t verticesCount);
	~VertexBuffer();

	void Bind();
	void Unbind();

private:
	uint32_t m_ID;
};

class ElementBuffer
{
public:
	ElementBuffer(uint32_t* indices, size_t indicesSize);
	~ElementBuffer();

	void Bind();
	void Unbind();

private:
	uint32_t m_ID;

};
