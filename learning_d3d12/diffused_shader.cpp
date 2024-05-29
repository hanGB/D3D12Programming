#include "stdafx.h"
#include "diffused_shader.h"

DiffusedShader::DiffusedShader(const wchar_t* vertex, const wchar_t* pixel)
	: Shader(vertex, pixel)
{
}

DiffusedShader::~DiffusedShader()
{
}

void DiffusedShader::CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature)
{
	m_numPipelineStaes = 1;
	m_pipelineStates = new ID3D12PipelineState * [m_numPipelineStaes];
	m_pipelineStates[0] = BuildPipelineState(device, rootSignature);
}

D3D12_INPUT_LAYOUT_DESC DiffusedShader::SetAndGetInputLayoutDesc()
{
	UINT numInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs
		= new D3D12_INPUT_ELEMENT_DESC[numInputElementDescs];

	// 버텍스는 위치와 색상을 가짐
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
		0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	::ZeroMemory(&inputLayoutDesc, sizeof(D3D12_INPUT_LAYOUT_DESC));
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = numInputElementDescs;

	return inputLayoutDesc;
}
