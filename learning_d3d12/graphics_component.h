#pragma once
#include "per_component.h"
#include "d3d12_mesh.h"
#include "d3d12_shader.h"
#include "d3d12_camera.h"

class GraphicsComponent : public PERComponent {
public:
	GraphicsComponent();
	virtual ~GraphicsComponent();
	virtual void Initialize();

	virtual void Update(float dTime);
	virtual void Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera);

	void SetMesh(d3d12_mesh::Mesh* mesh);
	void SetShader(d3d12_shader::Shader* shader);

	void ReleaseUploadBuffers();

	virtual void CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* commandList);
	virtual void ReleaseShaderVariables();

	void SetIsLiving(bool live);
	bool GetIsLiving() const;

protected:
	XMFLOAT4X4 m_worldTransform;

	d3d12_mesh::Mesh* m_mesh;
	d3d12_shader::Shader* m_shader;

	bool m_isLiving = false;
};