#include "stdafx.h"
#include "d3d12_pipleline.h"

d3d12_init::PipelineBuilder::PipelineBuilder(ID3D12Device* device)
{
	m_device = device;
	m_rootSignature = NULL;
	m_pipelineState = NULL;
}

d3d12_init::PipelineBuilder::~PipelineBuilder()
{
}

void d3d12_init::PipelineBuilder::Build()
{
	CreateRootSignature();
	CreatePipelineState();
}

ID3D12RootSignature* d3d12_init::PipelineBuilder::GetRootSignature()
{
	return m_rootSignature;
}

ID3D12PipelineState* d3d12_init::PipelineBuilder::GetPipelineState()
{
	return m_pipelineState;
}

void d3d12_init::PipelineBuilder::CreateRootSignature()
{
	SetRootSignatureDesc();
	MakeSignatureBlob();

	m_device->CreateRootSignature(0, m_rootSignatureBlob->GetBufferPointer(),
		m_rootSignatureBlob->GetBufferSize(),
		__uuidof(ID3D12RootSignature), (void**)&m_rootSignature);

	if (m_rootSignatureBlob) m_rootSignatureBlob->Release();
	if (m_errorBlob) m_errorBlob->Release();
}

void d3d12_init::PipelineBuilder::CreatePipelineState()
{
	CompileShader();
	SetRasterizerDesc();
	SetBlendDesc();
	SetPipelineStateDesc();

	m_device->CreateGraphicsPipelineState(&m_pipelineStateDesc,
		__uuidof(ID3D12PipelineState), (void**)&m_pipelineState);

	if (m_vertexShaderBlob) m_vertexShaderBlob->Release();
	if (m_pixelShaderBlob) m_pixelShaderBlob->Release();
}

void d3d12_init::PipelineBuilder::SetVetexShaderName(const wchar_t* name)
{
	m_vertexShaderName = name;
}

void d3d12_init::PipelineBuilder::SetPixelShaderName(const wchar_t* name)
{
	m_pixelShaderName = name;
}

void d3d12_init::PipelineBuilder::SetRootSignatureDesc()
{
	::ZeroMemory(&m_rootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	m_rootSignatureDesc.NumParameters = 0;
	m_rootSignatureDesc.pParameters = NULL;
	m_rootSignatureDesc.NumStaticSamplers = 0;
	m_rootSignatureDesc.pStaticSamplers = NULL;
	m_rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
}

void d3d12_init::PipelineBuilder::MakeSignatureBlob()
{
	::D3D12SerializeRootSignature(&m_rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&m_rootSignatureBlob, &m_errorBlob);
}

void d3d12_init::PipelineBuilder::CompileShader()
{
	UINT compileFlags = 0;
#ifdef PER_DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif 
	HRESULT result;

	result = D3DCompileFromFile(m_vertexShaderName.c_str(), NULL, NULL, "main", "vs_5_1", 
		compileFlags, 0, &m_vertexShaderBlob, NULL);
	result = D3DCompileFromFile(m_pixelShaderName.c_str(), NULL, NULL, "main", "ps_5_1",
		compileFlags, 0, &m_pixelShaderBlob, NULL);
}

void d3d12_init::PipelineBuilder::SetRasterizerDesc()
{
	::ZeroMemory(&m_rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	m_rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	m_rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	m_rasterizerDesc.FrontCounterClockwise = FALSE;
	m_rasterizerDesc.DepthBias = 0;
	m_rasterizerDesc.DepthBiasClamp = 0.0f;
	m_rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	m_rasterizerDesc.DepthClipEnable = TRUE;
	m_rasterizerDesc.MultisampleEnable = FALSE;
	m_rasterizerDesc.AntialiasedLineEnable = FALSE;
	m_rasterizerDesc.ForcedSampleCount = 0;
	m_rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

void d3d12_init::PipelineBuilder::SetBlendDesc()
{
	::ZeroMemory(&m_blendDesc, sizeof(D3D12_BLEND_DESC));
	m_blendDesc.AlphaToCoverageEnable = FALSE;
	m_blendDesc.IndependentBlendEnable = FALSE;
	m_blendDesc.RenderTarget[0].BlendEnable = FALSE;
	m_blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	m_blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	m_blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	m_blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	m_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	m_blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	m_blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	m_blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	m_blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void d3d12_init::PipelineBuilder::SetPipelineStateDesc()
{
	::ZeroMemory(&m_pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_pipelineStateDesc.pRootSignature = m_rootSignature;
	m_pipelineStateDesc.VS.pShaderBytecode = m_vertexShaderBlob->GetBufferPointer();
	m_pipelineStateDesc.VS.BytecodeLength = m_vertexShaderBlob->GetBufferSize();
	m_pipelineStateDesc.PS.pShaderBytecode = m_pixelShaderBlob->GetBufferPointer();
	m_pipelineStateDesc.PS.BytecodeLength = m_pixelShaderBlob->GetBufferSize();
	m_pipelineStateDesc.RasterizerState = m_rasterizerDesc;
	m_pipelineStateDesc.BlendState = m_blendDesc;
	m_pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	m_pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	m_pipelineStateDesc.InputLayout.pInputElementDescs = NULL;
	m_pipelineStateDesc.InputLayout.NumElements = 0;
	m_pipelineStateDesc.SampleMask = UINT_MAX;
	m_pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_pipelineStateDesc.NumRenderTargets = 1;
	m_pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_pipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_pipelineStateDesc.SampleDesc.Count = 1;
	m_pipelineStateDesc.SampleDesc.Quality = 0;
}
