#pragma once

class D3D12Camera;
class ResourceStorage;

namespace d3d12_shader {
	struct VS_VB_INSTNACE
	{
		XMFLOAT4X4	transform;
		XMFLOAT4	color;
	};


	class Shader {
	public:
		Shader(const wchar_t* vertex, const wchar_t* pixel);
		virtual ~Shader();

		virtual void CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature);

		void AddRef();
		void Release();

		void ReleaseUploadBuffers();

		void OnPrepareRender(ID3D12GraphicsCommandList* commandList);

		virtual void Render(ResourceStorage& resourceStorage, ID3D12GraphicsCommandList* commandList, D3D12Camera* camera);

		virtual void CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* commandList);
		virtual void ReleaseShaderVariables();
		virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* commandList, XMFLOAT4X4* modelTransform);

	protected:
		ID3D12PipelineState* BuildPipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature);

		D3D12_SHADER_BYTECODE LoadShader(const wchar_t* shaderFile, ID3DBlob** shaderBlob);

		virtual D3D12_RASTERIZER_DESC SetAndGetRasterizerDesc();
		virtual D3D12_BLEND_DESC SetAndGetBlendDesc();
		virtual D3D12_DEPTH_STENCIL_DESC SetAndGetDepthStencilDesc();
		virtual D3D12_INPUT_LAYOUT_DESC SetAndGetInputLayoutDesc();

		virtual D3D12_GRAPHICS_PIPELINE_STATE_DESC SetAndGetPipelineStateDesc(
			ID3D12RootSignature* rootSignature,
			const wchar_t* vertexShaderFile, ID3DBlob** vertexShaderBlob,
			const wchar_t* pixelShaderFile, ID3DBlob** pixelShaderBlob);

		ID3D12PipelineState** m_pipelineStates = NULL;
		int m_numPipelineStaes = 0;

	private:
		std::wstring m_vertex, m_pixel;

		int m_numReferences = 0;
	};
}