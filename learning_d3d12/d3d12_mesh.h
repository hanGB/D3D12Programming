#pragma once

namespace d3d12_mesh {

	class Mesh
	{
	public:
		Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		virtual ~Mesh();

		virtual void Render(ID3D12GraphicsCommandList* commandList);

		void AddRef();
		void Release();

		void ReleaseUploadBuffers();

	protected:
		ID3D12Resource* m_vertexBuffer = NULL;
		ID3D12Resource* m_vertexUploadBuffer = NULL;

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		UINT m_slot = 0;
		UINT m_numVertices = 0;
		UINT m_stride = 0;
		UINT m_offSet = 0;

	private:
		int m_numReferences = 0;
	};

	class TriangleMesh : public Mesh {
	public:
		TriangleMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		~TriangleMesh();
	};

	class CubeMeshDiffused : public Mesh {
	public:
		CubeMeshDiffused(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
			float width = 2.0f, float height = 2.0f, float depth = 2.0f);
		~CubeMeshDiffused();
	};
}