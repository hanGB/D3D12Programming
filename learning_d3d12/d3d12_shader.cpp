#include "stdafx.h"
#include "d3d12_shader.h"


d3d12_shader::Shader::Shader(const wchar_t* vertex, const wchar_t* pixel)
{
	m_pipelineBuilder = new d3d12_init::PipelineBuilder();
	m_pipelineBuilder->SetVertexShader(vertex);
	m_pipelineBuilder->SetPixelShader(pixel);

	m_numPipelineStaes = 0;
	m_numReferences = 0;
}

d3d12_shader::Shader::~Shader()
{
	for (int i = 0; i < m_numPipelineStaes; ++i) {
		m_pipelineStates[i]->Release();
	}
	delete[] m_pipelineStates;
	delete m_pipelineBuilder;
}

void d3d12_shader::Shader::CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature)
{
	m_numPipelineStaes = 1;
	m_pipelineStates = new ID3D12PipelineState*[m_numPipelineStaes];
	m_pipelineStates[0] = m_pipelineBuilder->Build(device, rootSignature);
}

void d3d12_shader::Shader::AddRef()
{
	m_numReferences++;
}

void d3d12_shader::Shader::Release()
{
	if (--m_numReferences <= 0) delete this;
}

void d3d12_shader::Shader::ReleaseUploadBuffers()
{

}

void d3d12_shader::Shader::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetPipelineState(m_pipelineStates[0]);
}
