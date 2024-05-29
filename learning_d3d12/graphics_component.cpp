#include "stdafx.h"
#include "graphics_component.h"
#include "per_object.h"

GraphicsComponent::GraphicsComponent()
{
}

GraphicsComponent::~GraphicsComponent()
{
	if (m_shader) m_shader->Release();
	if (m_mesh) m_mesh->Release();

	PERComponent* next = GetNext();
	if (next) delete next;
}

void GraphicsComponent::Initialize()
{
	if (GetNext()) GetNext()->Initialize();
}

void GraphicsComponent::Update(float dTime)
{
	m_modelTransform = GetOwner()->GetModelTransform();
}

void GraphicsComponent::Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera)
{
	if (m_shader)
	{
		m_shader->UpdateShaderVariable(commandList, &m_modelTransform);
		m_shader->Render(commandList, camera);
	}
	if (m_mesh) m_mesh->Render(commandList);
}

void GraphicsComponent::SetMesh(d3d12_mesh::Mesh* mesh)
{
	if (m_mesh) m_mesh->Release();
	m_mesh = mesh;
	if (m_mesh) m_mesh->AddRef();
}

void GraphicsComponent::SetShader(d3d12_shader::Shader* shader)
{
	if (m_shader) m_shader->Release();
	m_shader = shader;
	if (m_shader) m_shader->AddRef();
}

void GraphicsComponent::ReleaseUploadBuffers()
{
	if (m_mesh) m_mesh->ReleaseUploadBuffers();
}
