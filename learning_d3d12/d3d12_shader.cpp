#include "stdafx.h"
#include "d3d12_shader.h"
#include "d3d12_read_file.h"


d3d12_shader::Shader::Shader(const wchar_t* vertex, const wchar_t* pixel)
{
	m_vertex = vertex;
	m_pixel = pixel;

	m_numPipelineStaes = 0;
	m_numReferences = 0;
}

d3d12_shader::Shader::~Shader()
{
	for (int i = 0; i < m_numPipelineStaes; ++i) {
		m_pipelineStates[i]->Release();
	}
	delete[] m_pipelineStates;
}

void d3d12_shader::Shader::CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature)
{
	m_numPipelineStaes = 1;
	m_pipelineStates = new ID3D12PipelineState*[m_numPipelineStaes];
	m_pipelineStates[0] = BuildPipelineState(device, rootSignature);
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

void d3d12_shader::Shader::OnPrepareRender(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetPipelineState(m_pipelineStates[0]);
}

void d3d12_shader::Shader::Render(ID3D12GraphicsCommandList* commandList, D3D12Camera* camera)
{
	OnPrepareRender(commandList);
}

void d3d12_shader::Shader::CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
}

void d3d12_shader::Shader::UpdateShaderVariables(ID3D12GraphicsCommandList* commandList)
{
}

void d3d12_shader::Shader::ReleaseShaderVariables()
{
}

void d3d12_shader::Shader::UpdateShaderVariable(ID3D12GraphicsCommandList* commandList, XMFLOAT4X4* modelTransform)
{
	XMFLOAT4X4 model;
	XMStoreFloat4x4(&model, XMMatrixTranspose(XMLoadFloat4x4(modelTransform)));
	commandList->SetGraphicsRoot32BitConstants(0, 16, &model, 0);
}

ID3D12PipelineState* d3d12_shader::Shader::BuildPipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature)
{
	ID3D12PipelineState* pipelineState = NULL;
	ID3DBlob* vertexShaderBlob = NULL;
	ID3DBlob* pixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDec =
		SetAndGetPipelineStateDesc(rootSignature,
			m_vertex.c_str(), &vertexShaderBlob,
			m_pixel.c_str(), &pixelShaderBlob);

	HRESULT result = device->CreateGraphicsPipelineState(&pipelineStateDec,
		__uuidof(ID3D12PipelineState), (void**)&pipelineState);

	// 삭제 필요한 거 삭제
	vertexShaderBlob->Release();
	pixelShaderBlob->Release();
	
	if (pipelineStateDec.InputLayout.pInputElementDescs) delete[] pipelineStateDec.InputLayout.pInputElementDescs;
	m_vertex.clear();
	m_pixel.clear();
	m_vertex.shrink_to_fit();
	m_pixel.shrink_to_fit();

	return pipelineState;
}

D3D12_SHADER_BYTECODE d3d12_shader::Shader::LoadShader(const wchar_t* shaderFile, ID3DBlob** shaderBlob)
{
	*shaderBlob = d3d12_util::ReadCsoToBuffer(shaderFile);

	D3D12_SHADER_BYTECODE shaderCode;
	shaderCode.pShaderBytecode = (*shaderBlob)->GetBufferPointer();
	shaderCode.BytecodeLength = (*shaderBlob)->GetBufferSize();

	return shaderCode;
}

D3D12_RASTERIZER_DESC d3d12_shader::Shader::SetAndGetRasterizerDesc()
{
	D3D12_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_BLEND_DESC d3d12_shader::Shader::SetAndGetBlendDesc()
{
	D3D12_BLEND_DESC blendDesc;
	::ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_DEPTH_STENCIL_DESC d3d12_shader::Shader::SetAndGetDepthStencilDesc()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	::ZeroMemory(&depthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_INPUT_LAYOUT_DESC d3d12_shader::Shader::SetAndGetInputLayoutDesc()
{
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	::ZeroMemory(&inputLayoutDesc, sizeof(D3D12_INPUT_LAYOUT_DESC));
	inputLayoutDesc.pInputElementDescs = NULL;
	inputLayoutDesc.NumElements = 0;

	return inputLayoutDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12_shader::Shader::SetAndGetPipelineStateDesc(
	ID3D12RootSignature* rootSignature, const wchar_t* vertexShaderFile, ID3DBlob** vertexShaderBlob, 
	const wchar_t* pixelShaderFile, ID3DBlob** pixelShaderBlob)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;
	::ZeroMemory(&pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	pipelineStateDesc.pRootSignature = rootSignature;
	pipelineStateDesc.VS = LoadShader(vertexShaderFile, vertexShaderBlob);
	pipelineStateDesc.PS = LoadShader(pixelShaderFile, pixelShaderBlob);
	pipelineStateDesc.RasterizerState = SetAndGetRasterizerDesc();
	pipelineStateDesc.BlendState = SetAndGetBlendDesc();
	pipelineStateDesc.DepthStencilState = SetAndGetDepthStencilDesc();
	pipelineStateDesc.InputLayout = SetAndGetInputLayoutDesc();
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;

	return pipelineStateDesc;
}
