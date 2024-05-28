#pragma once

namespace d3d12_init {

	// 루트 시그니쳐
	D3D12_ROOT_SIGNATURE_DESC SetAndGetRootSignatureDesc();
	ID3D12RootSignature* CreateRootSignature(ID3D12Device* device);

	class PipelineBuilder {
	public:
		PipelineBuilder();
		~PipelineBuilder();

		void SetVertexShader(const wchar_t* vertexShaderFile);
		void SetPixelShader(const wchar_t* pixelShaderFile);

		ID3D12PipelineState* Build(ID3D12Device* device, ID3D12RootSignature* rootSignature);

	private:
		virtual D3D12_GRAPHICS_PIPELINE_STATE_DESC SetAndGetPipelineStateDesc(
			ID3D12RootSignature* rootSignature,
			const wchar_t* vertexShaderFile, ID3DBlob** vertexShaderBlob,
			const wchar_t* pixelShaderFile, ID3DBlob** pixelShaderBlob);

		D3D12_SHADER_BYTECODE CreateVertexShader(const wchar_t* vertexShaderFile, ID3DBlob** vertexShaderBlob);
		D3D12_SHADER_BYTECODE CreatePixelShader(const wchar_t* pixelShaderFile, ID3DBlob** pixelShaderBlob);
		D3D12_SHADER_BYTECODE LoadShader(const wchar_t* shaderFile, ID3DBlob** shaderBlob);

		virtual D3D12_RASTERIZER_DESC SetAndGetRasterizerDesc();
		virtual D3D12_BLEND_DESC SetAndGetBlendDesc();
		virtual D3D12_DEPTH_STENCIL_DESC SetAndGetDepthStencilDesc();
		virtual D3D12_INPUT_LAYOUT_DESC SetAndGetInputLayoutDesc();

		D3D12_SHADER_BYTECODE CompileShader(const wchar_t* shaderFile, LPCSTR shaderMainFuncName, LPCSTR shaderPropile, ID3DBlob** shaderBlob);

		// 파이프라인 스테이트를 만드는데 필요한 변수
		std::wstring m_vertexShaderName;
		std::wstring m_pixelShaderName;
	};
}