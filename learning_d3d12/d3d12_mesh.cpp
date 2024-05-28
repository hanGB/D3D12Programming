#include "stdafx.h"
#include "d3d12_mesh.h"
#include "d3d12_vertex.h"
#include "d3d12_resource.h"

d3d12_mesh::Mesh::Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
}

d3d12_mesh::Mesh::~Mesh()
{
	if (m_vertexBuffer) m_vertexBuffer->Release();
	if (m_vertexUploadBuffer) m_vertexUploadBuffer->Release();
}

void d3d12_mesh::Mesh::Render(ID3D12GraphicsCommandList* commandList)
{
	// 프리미티브 유형 설정
	commandList->IASetPrimitiveTopology(m_primitiveTopology);
	// 버텍스 버퍼 뷰 설정
	commandList->IASetVertexBuffers(m_slot, 1, &m_vertexBufferView);
	// 버텍스 버퍼 뷰 렌더링
	commandList->DrawInstanced(m_numVertices, 1, m_offSet, 0);
}

void d3d12_mesh::Mesh::AddRef()
{
	m_numReferences++;
}

void d3d12_mesh::Mesh::Release()
{
	if (--m_numReferences <= 0) delete this;
}

void d3d12_mesh::Mesh::ReleaseUploadBuffers()
{
	if (m_vertexUploadBuffer) m_vertexUploadBuffer->Release();
	m_vertexUploadBuffer = NULL;
}

d3d12_mesh::TriangleMesh::TriangleMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	: Mesh(device, commandList)
{
	// 삼각형 메쉬 정의
	m_numVertices = 3;
	m_stride = sizeof(DiffusedVertex);
	m_primitiveTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 버텍스 설정
	DiffusedVertex vertices[3];
	vertices[0] = DiffusedVertex(XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	vertices[1] = DiffusedVertex(XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	vertices[2] = DiffusedVertex(XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));

	// 삼각형 메쉬를 리소스로 생성
	m_vertexBuffer = d3d12_init::CreateBufferResource(device, commandList, 
		vertices, m_stride * m_numVertices, 
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 
		&m_vertexUploadBuffer);

	// 버텍스 버퍼 뷰 생성
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = m_stride;
	m_vertexBufferView.SizeInBytes = m_stride * m_numVertices; 
}

d3d12_mesh::TriangleMesh::~TriangleMesh()
{
}
