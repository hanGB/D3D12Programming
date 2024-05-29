#pragma once
#include "d3d12_shader.h"

class DiffusedShader : public d3d12_shader::Shader {
public:
	DiffusedShader(const wchar_t* vertex, const wchar_t* pixel);
	virtual ~DiffusedShader();

	virtual void CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature);

protected:
	virtual D3D12_INPUT_LAYOUT_DESC SetAndGetInputLayoutDesc();
};