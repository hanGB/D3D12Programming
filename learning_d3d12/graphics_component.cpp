#include "stdafx.h"
#include "graphics_component.h"
#include "per_object.h"
#include "camera_component.h"

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
	m_isLiving = false;

	PERComponent::Initialize();
}

void GraphicsComponent::Update(float dTime)
{
	m_worldTransform = GetOwner()->GetWorldTransform();

	if (GetNext()) dynamic_cast<GraphicsComponent*>(GetNext())->Update(dTime);
}

void GraphicsComponent::Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera, UINT numInstances)
{
	UpdateShaderVariables(commandList);

	if (m_shader) m_shader->Render(commandList, camera);

	if (m_mesh) m_mesh->Render(commandList, numInstances);

	if (GetNext()) dynamic_cast<GraphicsComponent*>(GetNext())->Render(commandList, camera, numInstances);
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
	
}

void GraphicsComponent::CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	D3D12Camera* camera = GetOwner()->GetComponentWithType<CameraComponent>(nullptr)->GetCamera();
	if (camera) camera->CreateShaderVariables(device, commandList);
}

void GraphicsComponent::UpdateShaderVariables(ID3D12GraphicsCommandList* commandList)
{
	XMFLOAT4X4 transposedModel;
	XMStoreFloat4x4(&transposedModel, XMMatrixTranspose(XMLoadFloat4x4(&m_worldTransform)));

	if (m_mesh && m_mesh->IsHaveToRotate()) {
		transposedModel = Matrix4x4::Multiply(transposedModel, m_mesh->GetDefaultRotation());
	}
	commandList->SetGraphicsRoot32BitConstants(0, 16, &transposedModel, 0);
}

void GraphicsComponent::ReleaseShaderVariables()
{
}

void GraphicsComponent::SetIsLiving(bool live)
{
	m_isLiving = live;
}

bool GraphicsComponent::GetIsLiving() const
{
	return m_isLiving;
}

XMFLOAT4X4 GraphicsComponent::GetWorldTransform() const
{
	return m_worldTransform;
}
