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
		// 베텍스
		ID3D12Resource* m_vertexBuffer = NULL;
		ID3D12Resource* m_vertexUploadBuffer = NULL;

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		UINT m_slot = 0;
		UINT m_numVertices = 0;
		UINT m_stride = 0;
		UINT m_offSet = 0;

		// 인덱스
		ID3D12Resource* m_indexBuffer = NULL;
		ID3D12Resource* m_indexUploadBuffer = NULL;

		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		UINT m_numIndices = 0;
		UINT m_startIndex = 0;
		int m_numBaseVertex = 0;

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

	class AirplaneMeshDiffused : public Mesh {
	public:
		AirplaneMeshDiffused(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
			float width = 20.0f, float height = 20.0f, float depth = 4.0f, XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 0.0, 0.0f));
		~AirplaneMeshDiffused();
	};
}