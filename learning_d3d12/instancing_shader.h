#pragma once
#include "graphics_components_shader.h"
#include "d3d12_camera.h"

class InstancingShader : public GraphicsComponentsShader {
public:
	InstancingShader(const wchar_t* vertex, const wchar_t* pixel);
	virtual ~InstancingShader();

	virtual void CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature);

	virtual void Render(ResourceStorage& resourceStorage, ID3D12GraphicsCommandList* commandList, D3D12Camera* camera);

	virtual void CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* commandList);
	virtual void ReleaseShaderVariables();

protected:
	virtual D3D12_INPUT_LAYOUT_DESC SetAndGetInputLayoutDesc();

	ID3D12Resource* m_cbGraphicsComponents = NULL;
	d3d12_shader::VS_VB_INSTNACE* m_cbMappedGraphicsComponents = NULL;
};