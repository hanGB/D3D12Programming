#include "stdafx.h"
#include "d3d12_vertex.h"

d3d12_mesh::Vertex::Vertex()
{
	m_position = XMFLOAT3(0.f, 0.f, 0.f);
}

d3d12_mesh::Vertex::Vertex(XMFLOAT3 position)
{
	m_position = position;
}

d3d12_mesh::Vertex::~Vertex()
{
}

d3d12_mesh::DiffusedVertex::DiffusedVertex()
{
	m_position = XMFLOAT3(0.f, 0.f, 0.f);
	m_diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
}

d3d12_mesh::DiffusedVertex::DiffusedVertex(XMFLOAT3 position, XMFLOAT4 diffuse)
{
	m_position = position;
	m_diffuse = diffuse;
}

d3d12_mesh::DiffusedVertex::~DiffusedVertex()
{
}
