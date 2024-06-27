#include "stdafx.h"
#include "instancing_shader.h"
#include "d3d12_resource.h"

InstancingShader::InstancingShader(const wchar_t* vertex, const wchar_t* pixel)
	: GraphicsComponentsShader(vertex, pixel)
{
}

InstancingShader::~InstancingShader()
{
}

void InstancingShader::CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature)
{
	m_numPipelineStaes = 1;
	m_pipelineStates = new ID3D12PipelineState*[m_numPipelineStaes];
	m_pipelineStates[0] = BuildPipelineState(device, rootSignature);
}

void InstancingShader::Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera)
{
	// 렌더링
	d3d12_shader::Shader::Render(commandList, camera);
	
	// 인스턴싱 데이터를 버퍼에 저장
	UpdateShaderVariables(commandList);

	m_graphicsComponents[0]->Render(commandList, camera, m_numComponents);
}

void InstancingShader::CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	// 인스턴스 정보를 저장할 정점 버퍼를 업로드 할 힙 생성
	m_cbGraphicsComponents = d3d12_init::CreateBufferResource(device, commandList, 
		NULL, sizeof(d3d12_shader::VS_VB_INSTNACE) * m_numComponents,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	// 버텍스 버퍼(업로드 힙)에 대한 포인터 저장
	m_cbGraphicsComponents->Map(0, NULL, (void**)&m_cbMappedGraphicsComponents);
}

void InstancingShader::UpdateShaderVariables(ID3D12GraphicsCommandList* commandList)
{
	DoGarbegeCollection();

	commandList->SetGraphicsRootShaderResourceView(2, m_cbGraphicsComponents->GetGPUVirtualAddress());
	for (int i = 0; i < m_numComponents; ++i) {
		m_cbMappedGraphicsComponents[i].color = (i % 2) ? XMFLOAT4(0.5f, 0.0f, 0.0f, 0.0f) : XMFLOAT4(0.0f, 0.0f, 0.5f, 0.0f);
		XMFLOAT4X4 worldTransform = m_graphicsComponents[i]->GetWorldTransform();
		XMStoreFloat4x4(&m_cbMappedGraphicsComponents[i].transform, XMMatrixTranspose(XMLoadFloat4x4(&worldTransform)));
	}
}

void InstancingShader::ReleaseShaderVariables()
{
	if (m_cbGraphicsComponents) m_cbGraphicsComponents->Unmap(0, NULL);
	if (m_cbGraphicsComponents) m_cbGraphicsComponents->Release();
}

D3D12_INPUT_LAYOUT_DESC InstancingShader::SetAndGetInputLayoutDesc()
{
	UINT numInputElementsDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[numInputElementsDescs];

	// 버텍스 정보 입력
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = numInputElementsDescs;

	return inputLayoutDesc;
}
