#pragma once
#include "per_component.h"
#include "d3d12_mesh.h"
#include "d3d12_shader.h"

class GraphicsComponent : public PERComponent {
public:
	GraphicsComponent();
	virtual ~GraphicsComponent();
	virtual void Initialize();

	virtual void Update(float dTime);
	virtual void Render(ID3D12GraphicsCommandList* commandList);

	void SetMesh(d3d12_mesh::Mesh* mesh);
	void SetShader(d3d12_shader::Shader* shader);

	void ReleaseUploadBuffers();

protected:
	XMFLOAT4X4 m_worldTransform;

	d3d12_mesh::Mesh* m_mesh;
	d3d12_shader::Shader* m_shader;
};