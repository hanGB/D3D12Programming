#pragma once
#include "per_component.h"
#include "d3d12_mesh.h"
#include "d3d12_shader.h"
#include "d3d12_camera.h"

class ResourceStorage;

class GraphicsComponent : public PERComponent {
public:
	GraphicsComponent();
	virtual ~GraphicsComponent();
	virtual void Initialize();

	virtual void Update(float dTime);
	virtual void Render(ResourceStorage& resourceStorage, ID3D12GraphicsCommandList* commandList, D3D12Camera* camera, UINT numInstances);

	void SetMeshType(int meshType);
	void SetShader(d3d12_shader::Shader* shader);

	int GetMeshType() const;

	void ReleaseUploadBuffers();

	virtual void CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	virtual void UpdateShaderVariables(d3d12_mesh::Mesh* mesh, ID3D12GraphicsCommandList* commandList);
	virtual void ReleaseShaderVariables();

	void SetIsLiving(bool live);
	bool GetIsLiving() const;

	XMFLOAT4X4 GetWorldTransform() const;

protected:
	XMFLOAT4X4 m_worldTransform;

	int m_meshType;

	d3d12_shader::Shader* m_shader;

	bool m_isLiving = false;
};