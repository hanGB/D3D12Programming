#include "stdafx.h"
#include "per_world.h"
#include "d3d12_root.h"
#include "per_object.h"
#include "graphics_component.h"
#include "physics_component.h"
#include "d3d12_camera.h"
#include "diffused_shader.h"

PERWorld::PERWorld()
{
	m_rootSignature = NULL;
	m_shader = NULL;
	m_mesh = NULL;
}

PERWorld::~PERWorld()
{
}

void PERWorld::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	// 루트 시그니처 생성
	m_rootSignature = d3d12_init::CreateRootSignature(device);

	// 쉐이더 생성
	m_shader = new DiffusedShader(L"./shader/vertex_shader.cso", L"./shader/pixel_shader.cso");
	m_shader->CreatePipelineState(device, m_rootSignature);

	m_mesh = new d3d12_mesh::CubeMeshDiffused(device, commandList, 12.0f, 12.0f, 12.0f);

	m_object = new PERObject(new PhysicsComponent(), new GraphicsComponent());
	m_object->GetGraphics().SetShader(m_shader);
	m_object->GetGraphics().SetMesh(m_mesh);
}

void PERWorld::ReleaseObjects()
{
	m_rootSignature->Release();
	m_shader->Release();
}

void PERWorld::SetCameraInformation(D3D12Camera* camera, int width, int height)
{
	camera->GenerateProjectionMatrix(90.0f, (float)width / (float)height, 0.1f, 300.0f);
	camera->GenerateViewMatrix(XMFLOAT3(0.0f, 15.0f, -25.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
}

void PERWorld::PhysicsUpdate(float deltaTime)
{
	m_object->GetPhysics().Update(deltaTime);
}
void PERWorld::GraphicsUpdate(float deltaTime)
{
	m_object->GetGraphics().Update(deltaTime);
}

void PERWorld::Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera)
{
	camera->SetViewportsAndScissorRect(commandList);
	commandList->SetGraphicsRootSignature(m_rootSignature);

	camera->UpdateShaderVariables(commandList);

	m_object->GetGraphics().Render(commandList, camera);
}

void PERWorld::ReleaseUploadBuffers()
{
	m_object->GetGraphics().ReleaseUploadBuffers();
}
