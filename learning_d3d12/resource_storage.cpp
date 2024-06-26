#include "stdafx.h"
#include "resource_storage.h"

ResourceStorage::ResourceStorage()
{
}

ResourceStorage::~ResourceStorage()
{
	for (auto& mesh : m_meshes) 
	{
		mesh.second->ReleaseUploadBuffers();
	}
}

bool ResourceStorage::CheckIfMeshExists(int meshType)
{
	if (m_meshes.find(meshType) == m_meshes.end()) return false;

	return true;
}

void ResourceStorage::AddMesh(int meshType, d3d12_mesh::Mesh* mesh)
{
	m_meshes.insert(std::pair<int, d3d12_mesh::Mesh*>(meshType, mesh));
}

d3d12_mesh::Mesh* ResourceStorage::GetMesh(int meshType)
{
	if (!CheckIfMeshExists(meshType)) return nullptr;
	return m_meshes[meshType];
}
