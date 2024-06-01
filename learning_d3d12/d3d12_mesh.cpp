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

	if (m_indexBuffer) m_indexBuffer->Release();
	if (m_indexUploadBuffer) m_indexUploadBuffer->Release();
}

void d3d12_mesh::Mesh::Render(ID3D12GraphicsCommandList* commandList)
{
	// 프리미티브 유형 설정
	commandList->IASetPrimitiveTopology(m_primitiveTopology);
	// 버텍스 버퍼 뷰 설정
	commandList->IASetVertexBuffers(m_slot, 1, &m_vertexBufferView);
	if (m_indexBuffer)
	{
		// 인덱스 버퍼 뷰 렌더링
		commandList->IASetIndexBuffer(&m_indexBufferView);
		commandList->DrawIndexedInstanced(m_numIndices, 1, 0, 0, 0);
	}
	else
	{
		// 버텍스 버퍼 뷰 렌더링
		commandList->DrawInstanced(m_numVertices, 1, m_offSet, 0);
	}
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

	if (m_indexUploadBuffer) m_indexUploadBuffer->Release();
	m_indexUploadBuffer = NULL;
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

d3d12_mesh::CubeMeshDiffused::CubeMeshDiffused(
	ID3D12Device* device, ID3D12GraphicsCommandList* commandList, 
	float width, float height, float depth)
	: Mesh(device, commandList)
{
	m_numVertices = 8;
	m_stride = sizeof(DiffusedVertex);
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float x = width * 0.5f; 
	float y = height * 0.5f; 
	float z = depth * 0.5f;

	// 버텍스 생성
	DiffusedVertex vertices[8];
	vertices[0] = DiffusedVertex(XMFLOAT3(-x, +y, -z), RANDOM_COLOR);
	vertices[1] = DiffusedVertex(XMFLOAT3(+x, +y, -z), RANDOM_COLOR);
	vertices[2] = DiffusedVertex(XMFLOAT3(+x, +y, +z), RANDOM_COLOR);
	vertices[3] = DiffusedVertex(XMFLOAT3(-x, +y, +z), RANDOM_COLOR);
	vertices[4] = DiffusedVertex(XMFLOAT3(-x, -y, -z), RANDOM_COLOR);
	vertices[5] = DiffusedVertex(XMFLOAT3(+x, -y, -z), RANDOM_COLOR);
	vertices[6] = DiffusedVertex(XMFLOAT3(+x, -y, +z), RANDOM_COLOR);
	vertices[7] = DiffusedVertex(XMFLOAT3(-x, -y, +z), RANDOM_COLOR);

	m_vertexBuffer = d3d12_init::CreateBufferResource(
		device, commandList, vertices, m_stride * m_numVertices, 
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_vertexUploadBuffer);

	// 버텍스 버퍼 뷰 생성
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = m_stride;
	m_vertexBufferView.SizeInBytes = m_stride * m_numVertices;

	// 인덱스 버퍼
	m_numIndices = 36;

	UINT indices[36];
	//Front 사각형의 위쪽 삼각형
	indices[0] = 3; indices[1] = 1; indices[2] = 0;
	//Front 사각형의 아래쪽 삼각형
	indices[3] = 2; indices[4] = 1; indices[5] = 3;
	//Top 사각형의 위쪽 삼각형
	indices[6] = 0; indices[7] = 5; indices[8] = 4;
	//Top 사각형의 아래쪽 삼각형
	indices[9] = 1; indices[10] = 5; indices[11] = 0;
	//Back 사각형의 위쪽 삼각형
	indices[12] = 3; indices[13] = 4; indices[14] = 7;
	//Back 사각형의 아래쪽 삼각형
	indices[15] = 0; indices[16] = 4; indices[17] = 3;
	//Bottom 사각형의 위쪽 삼각형
	indices[18] = 1; indices[19] = 6; indices[20] = 5;
	//Bottom 사각형의 아래쪽 삼각형
	indices[21] = 2; indices[22] = 6; indices[23] = 1;
	//Left 사각형의 위쪽 삼각형
	indices[24] = 2; indices[25] = 7; indices[26] = 6;
	//Left 사각형의 아래쪽 삼각형
	indices[27] = 3; indices[28] = 7; indices[29] = 2;
	//Right 사각형의 위쪽 삼각형
	indices[30] = 6; indices[31] = 4; indices[32] = 5;
	//Right 사각형의 아래쪽 삼각형
	indices[33] = 7; indices[34] = 4; indices[35] = 6;

	// 인덱스 버퍼 생성
	m_indexBuffer = d3d12_init::CreateBufferResource(
		device, commandList, indices, sizeof(UINT) * m_numIndices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_indexUploadBuffer);

	// 인덱스 뷰 생성
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(UINT) * m_numIndices;
}

d3d12_mesh::CubeMeshDiffused::~CubeMeshDiffused()
{
}

d3d12_mesh::AirplaneMeshDiffused::AirplaneMeshDiffused(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, 
	float width, float height, float depth, XMFLOAT4 color)
	: Mesh(device, commandList)
{
	m_numVertices = 24 * 3;
	m_stride = sizeof(DiffusedVertex);
	m_offSet = 0;
	m_slot = 0;
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float x = width * 0.5f;
	float y = height * 0.5f;
	float z = depth * 0.5f;

	DiffusedVertex vertices[24 * 3];

	float x1 = x * 0.2f, y1 = y * 0.2f, x2 = x * 0.1f, y3 = y * 0.3f, y2 = ((y1 - (y - y3)) / x1) * x2 + (y - y3);

	int i = 0;

	//비행기 메쉬의 위쪽 면
	XMFLOAT4 randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x2, +y2, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x, -y3, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x2, +y2, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x, -y3, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	//비행기 메쉬의 아래쪽 면
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x2, +y2, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x, -y3, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x2, +y2, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x, -y3, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	//비행기 메쉬의 오른쪽 면
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x2, +y2, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x2, +y2, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x2, +y2, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x2, +y2, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x2, +y2, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x, -y3, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x, -y3, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x2, +y2, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x, -y3, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	//비행기 메쉬의 뒤쪽/오른쪽 면
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x, -y3, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x, -y3, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x, -y3, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(+x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	//비행기 메쉬의 왼쪽 면
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x2, +y2, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, +(y + y3), +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x2, +y2, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x2, +y2, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x2, +y2, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x2, +y2, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x, -y3, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x2, +y2, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x, -y3, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x, -y3, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	//비행기 메쉬의 뒤쪽/왼쪽 면
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(0.0f, 0.0f, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x, -y3, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x1, -y1, -z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x, -y3, +z), Vector4::Add(color, randomColor)); randomColor = RANDOM_COLOR;
	vertices[i++] = DiffusedVertex(XMFLOAT3(-x, -y3, -z), Vector4::Add(color, randomColor)); 

	m_vertexBuffer = d3d12_init::CreateBufferResource(device, commandList, vertices, m_stride * m_numVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_vertexUploadBuffer);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = m_stride;
	m_vertexBufferView.SizeInBytes = m_stride * m_numVertices;
}

d3d12_mesh::AirplaneMeshDiffused::~AirplaneMeshDiffused()
{
}
