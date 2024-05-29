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

protected:
	XMFLOAT4X4 m_modelTransform;

	d3d12_mesh::Mesh* m_mesh;
	d3d12_shader::Shader* m_shader;
};