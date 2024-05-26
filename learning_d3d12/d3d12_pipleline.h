#pragma once

namespace d3d12_init {

	class PipelineBuilder {
	public:
		PipelineBuilder(ID3D12Device* device);
		~PipelineBuilder();

		void Build();
		ID3D12RootSignature* GetRootSignature();
		ID3D12PipelineState* GetPipelineState();

		void SetVetexShaderName(const wchar_t* name);
		void SetPixelShaderName(const wchar_t* name);

	private:
		void CreateRootSignature();
		void CreatePipelineState();

		void SetRootSignatureDesc();
		void MakeSignatureBlob();

		void CompileShader();
		void SetRasterizerDesc();
		void SetBlendDesc();
		void SetPipelineStateDesc();

		ID3D12Device*	m_device;

		ID3D12RootSignature* m_rootSignature;
		ID3D12PipelineState* m_pipelineState;

		// 루트 시그니처를 만드는 데 필요한 변수
		D3D12_ROOT_SIGNATURE_DESC	m_rootSignatureDesc;
		ID3DBlob*					m_rootSignatureBlob = NULL;
		ID3DBlob*					m_errorBlob = NULL;

		// 파이프라인 스테이트를 만드는데 필요한 변수
		std::wstring						m_vertexShaderName;
		std::wstring						m_pixelShaderName;
		ID3DBlob*							m_vertexShaderBlob = NULL;
		ID3DBlob*							m_pixelShaderBlob = NULL;
		D3D12_RASTERIZER_DESC				m_rasterizerDesc;
		D3D12_BLEND_DESC					m_blendDesc;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_pipelineStateDesc;
	};
}