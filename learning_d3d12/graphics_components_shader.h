#pragma once
#include "d3d12_shader.h"
#include "graphics_component.h"
#include "d3d12_camera.h"

class GraphicsComponentsShader : public d3d12_shader::Shader {
public:
	GraphicsComponentsShader(const wchar_t* vertex, const wchar_t* pixel);
	virtual ~GraphicsComponentsShader();

	virtual void CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature);

	virtual void Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera);

	void AddGraphicsComponent(GraphicsComponent* component);

protected:
	virtual D3D12_INPUT_LAYOUT_DESC SetAndGetInputLayoutDesc();

	void DoGarbegeCollection();

	static const int c_INITIAL_MAXIMUM_COMPONENTS = 1024;

	std::vector<GraphicsComponent*> m_graphicsComponents;
	int m_numComponents = 0;
	int m_maximumComponents = c_INITIAL_MAXIMUM_COMPONENTS;
};