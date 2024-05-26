#include "stdafx.h"
#include "per_world.h"
#include "d3d12_pipleline.h"

PERWorld::PERWorld()
{
	m_rootSignature = NULL;
	m_pipelineState = NULL;
}

PERWorld::~PERWorld()
{
}

void PERWorld::BuildObjects(ID3D12Device* device)
{
	d3d12_init::PipelineBuilder pipelineBuilder(device);

	pipelineBuilder.SetVetexShaderName(L"vertex_shader.hlsl");
	pipelineBuilder.SetPixelShaderName(L"pixel_shader.hlsl");
	pipelineBuilder.Build();
	m_rootSignature = pipelineBuilder.GetRootSignature();
	m_pipelineState = pipelineBuilder.GetPipelineState();
}

void PERWorld::ReleaseObjects()
{
	m_rootSignature->Release();
	m_pipelineState->Release();
}

void PERWorld::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->SetPipelineState(m_pipelineState);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);
}
