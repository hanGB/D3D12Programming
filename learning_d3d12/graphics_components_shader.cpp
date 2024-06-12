#include "stdafx.h"
#include "graphics_components_shader.h"

GraphicsComponentsShader::GraphicsComponentsShader(const wchar_t* vertex, const wchar_t* pixel)
	:Shader(vertex, pixel)
{
	m_graphicsComponents.reserve(c_INITIAL_MAXIMUM_COMPONENTS);
	m_graphicsComponents.resize(c_INITIAL_MAXIMUM_COMPONENTS);
}

GraphicsComponentsShader::~GraphicsComponentsShader()
{
	m_graphicsComponents.clear();
	m_graphicsComponents.shrink_to_fit();
}

void GraphicsComponentsShader::CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature)
{
	m_numPipelineStaes = 1;
	m_pipelineStates = new ID3D12PipelineState * [m_numPipelineStaes];
	m_pipelineStates[0] = BuildPipelineState(device, rootSignature);
}

void GraphicsComponentsShader::Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera)
{
	// 렌더링 할 필요없는 죽은 컨포넌트 제거
	DoGarbegeCollection();

	d3d12_shader::Shader::Render(commandList, camera);
	for (int i = 0; i < m_numComponents; ++i)
	{
		m_graphicsComponents[i]->Render(commandList, camera, 1);
	}
}

void GraphicsComponentsShader::AddGraphicsComponent(GraphicsComponent* component)
{
	if (m_numComponents == m_maximumComponents) 
	{
		m_maximumComponents *= 2;
		m_graphicsComponents.reserve(m_maximumComponents);
		m_graphicsComponents.resize(m_maximumComponents);
	}
	component->SetIsLiving(true); // 임시로 shader에서 설정
	m_graphicsComponents[m_numComponents] = component;
	m_numComponents++;
}

D3D12_INPUT_LAYOUT_DESC GraphicsComponentsShader::SetAndGetInputLayoutDesc()
{
	UINT numInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[numInputElementDescs];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = numInputElementDescs;

	return inputLayoutDesc;
}

void GraphicsComponentsShader::DoGarbegeCollection()
{
	int index = 0;

	while (true) {
		for (index; index < m_numComponents; ++index) {
			if (!m_graphicsComponents[index]->GetIsLiving()) {
				m_graphicsComponents[index] = m_graphicsComponents[--m_numComponents];
				break;
			}
		}

		if (index >= m_numComponents) break;
	}
}
