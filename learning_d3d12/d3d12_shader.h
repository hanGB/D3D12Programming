#pragma once
#include "d3d12_pipleline.h"

namespace d3d12_shader {
	class Shader {
	public:
		Shader(const wchar_t* vertex, const wchar_t* pixel);
		virtual ~Shader();

		virtual void CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature);

		void AddRef();
		void Release();

		void ReleaseUploadBuffers();

		void Render(ID3D12GraphicsCommandList* commandList);

	protected:
		d3d12_init::PipelineBuilder* m_pipelineBuilder;

		ID3D12PipelineState** m_pipelineStates = NULL;
		int m_numPipelineStaes = 0;

	private:
		int m_numReferences = 0;
	};
}