#include "stdafx.h"
#include "per_world.h"
#include "d3d12_pipleline.h"
#include "per_object.h"
#include "graphics_component.h"

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
	m_shader = new d3d12_shader::Shader(L"./shader/vertex_shader.cso", L"./shader/pixel_shader.cso");
	m_shader->CreatePipelineState(device, m_rootSignature);

	m_mesh = new d3d12_mesh::TriangleMesh(device, commandList);

	m_object = new PERObject(new GraphicsComponent());
	m_object->GetGraphics().SetShader(m_shader);
	m_object->GetGraphics().SetMesh(m_mesh);
}

void PERWorld::ReleaseObjects()
{
	m_rootSignature->Release();
	m_shader->Release();
}

void PERWorld::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetGraphicsRootSignature(m_rootSignature);

	m_object->GetGraphics().Render(commandList);
}

void PERWorld::ReleaseUploadBuffers()
{
	m_object->GetGraphics().ReleaseUploadBuffers();
}
