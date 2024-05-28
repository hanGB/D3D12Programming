#pragma once

namespace d3d12_mesh {

	// 버텍스 표현용 클래스
	class Vertex {
	public:
		Vertex();
		Vertex(XMFLOAT3 position);
		~Vertex();

	protected:
		// 위치 벡터(버텍스은 최소한 위치 벡터 소유)
		XMFLOAT3 m_position;

	};

	class DiffusedVertex : public Vertex {
	public:
		DiffusedVertex();
		DiffusedVertex(XMFLOAT3 position, XMFLOAT4 diffuse);
		~DiffusedVertex();

	protected:
		// 디퓨즈 색상 벡터
		XMFLOAT4 m_diffuse;

	};

}
