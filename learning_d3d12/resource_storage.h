#pragma once
#include "d3d12_mesh.h"

class ResourceStorage {
public:
	ResourceStorage();
	~ResourceStorage();

	bool CheckIfMeshExists(int meshType);
	void AddMesh(int meshType, d3d12_mesh::Mesh* mesh);
	d3d12_mesh::Mesh* GetMesh(int meshType);

private:
	// 리소스 타입으로 메쉬, 텍스쳐를 얻기 추가

	std::unordered_map<int, d3d12_mesh::Mesh*> m_meshes;
};