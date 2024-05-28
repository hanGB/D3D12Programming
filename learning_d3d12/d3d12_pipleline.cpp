#include "stdafx.h"
#include "d3d12_pipleline.h"
#include "d3d12_read_file.h"

D3D12_ROOT_SIGNATURE_DESC d3d12_init::SetAndGetRootSignatureDesc()
{
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;

	::ZeroMemory(&rootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	rootSignatureDesc.NumParameters = 0;
	rootSignatureDesc.pParameters = NULL;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = NULL;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	return rootSignatureDesc;
}

ID3D12RootSignature* d3d12_init::CreateRootSignature(ID3D12Device* device)
{
	ID3D12RootSignature* rootSignature = nullptr;
	ID3DBlob* rootSignatureBlob, * errorBlob;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = d3d12_init::SetAndGetRootSignatureDesc();

	// Blob 생성
	::D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&rootSignatureBlob, &errorBlob);

	// 루트 시그니처 생성
	device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		__uuidof(ID3D12RootSignature), (void**)&rootSignature);

	// blob 삭제
	if (rootSignatureBlob) rootSignatureBlob->Release();
	if (errorBlob) errorBlob->Release();

	return rootSignature;
}


d3d12_init::PipelineBuilder::PipelineBuilder()
{
}

d3d12_init::PipelineBuilder::~PipelineBuilder()
{
}

void d3d12_init::PipelineBuilder::SetVertexShader(const wchar_t* vertexShaderFile)
{
	m_vertexShaderName = vertexShaderFile;
}

void d3d12_init::PipelineBuilder::SetPixelShader(const wchar_t* pixelShaderFile)
{
	m_pixelShaderName = pixelShaderFile;
}

ID3D12PipelineState* d3d12_init::PipelineBuilder::Build(ID3D12Device* device, ID3D12RootSignature* rootSignature)
{
	ID3D12PipelineState* pipelineState = NULL;
	ID3DBlob* vertexShaderBlob = NULL;
	ID3DBlob* pixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDec = 
		SetAndGetPipelineStateDesc(rootSignature,
		m_vertexShaderName.c_str(), &vertexShaderBlob,
		m_pixelShaderName.c_str(), &pixelShaderBlob);

	HRESULT result = device->CreateGraphicsPipelineState(&pipelineStateDec,
		__uuidof(ID3D12PipelineState), (void**)&pipelineState);

	// 삭제 필요한 거 삭제
	vertexShaderBlob->Release();
	pixelShaderBlob->Release();
	if (pipelineStateDec.InputLayout.pInputElementDescs) delete[] pipelineStateDec.InputLayout.pInputElementDescs;

	return pipelineState;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12_init::PipelineBuilder::SetAndGetPipelineStateDesc(
	ID3D12RootSignature* rootSignature,
	const wchar_t* vertexShaderFile, ID3DBlob** vertexShaderBlob,
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

\

D3D12_SHADER_BYTECODE d3d12_init::PipelineBuilder::CompileShader(const wchar_t* shaderFile, LPCSTR shaderMainFuncName, LPCSTR shaderPropile, ID3DBlob** shaderBlob)
{
	UINT compileFlags = 0;
#ifdef PER_DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif 

	ID3DBlob* error;

	HRESULT result = ::D3DCompileFromFile(shaderFile, NULL, NULL, shaderMainFuncName, shaderPropile,
		compileFlags, 0, shaderBlob, &error);

	D3D12_SHADER_BYTECODE shaderByteCode;
	shaderByteCode.BytecodeLength = (*shaderBlob)->GetBufferSize();
	shaderByteCode.pShaderBytecode = (*shaderBlob)->GetBufferPointer();

	return shaderByteCode;
}

D3D12_SHADER_BYTECODE d3d12_init::PipelineBuilder::CreateVertexShader(const wchar_t* vertexShaderFile, ID3DBlob** vertexShaderBlob)
{
	return CompileShader(vertexShaderFile, "main", "vs_5_1", vertexShaderBlob);
}

D3D12_SHADER_BYTECODE d3d12_init::PipelineBuilder::CreatePixelShader(const wchar_t* pixelShaderFile, ID3DBlob** pixelShaderBlob)
{
	return CompileShader(pixelShaderFile, "main", "ps_5_1", pixelShaderBlob);
}

D3D12_SHADER_BYTECODE d3d12_init::PipelineBuilder::LoadShader(const wchar_t* shaderFile, ID3DBlob** shaderBlob)
{
	*shaderBlob = d3d12_util::ReadCsoToBuffer(shaderFile);

	D3D12_SHADER_BYTECODE shaderCode;
	shaderCode.pShaderBytecode = (*shaderBlob)->GetBufferPointer();
	shaderCode.BytecodeLength = (*shaderBlob)->GetBufferSize();

	return shaderCode;
}

D3D12_RASTERIZER_DESC d3d12_init::PipelineBuilder::SetAndGetRasterizerDesc()
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

D3D12_BLEND_DESC d3d12_init::PipelineBuilder::SetAndGetBlendDesc()
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

D3D12_DEPTH_STENCIL_DESC d3d12_init::PipelineBuilder::SetAndGetDepthStencilDesc()
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

D3D12_INPUT_LAYOUT_DESC d3d12_init::PipelineBuilder::SetAndGetInputLayoutDesc()
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
