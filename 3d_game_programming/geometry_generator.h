#pragma once

class GeometryGenerator {
public:
	struct Vertex {
		Vertex();
		Vertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv)
			: position(p), normal(n), tangent(t), texCoord(uv)
		{

		}
		Vertex(
			float px, float py, float pz, float nx, float ny, float nz,
			float tx, float ty, float tz, float u, float v) :
			position(px, py, pz),
			normal(nx, ny, nz),
			tangent(tx, ty, tz),
			texCoord(u, v)
		{

		}

		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT2 texCoord;
	};

	struct MeshData {
		std::vector<uint16_t>& GetIndices16()
		{
			if (m_indices16.empty())
			{
				m_indices16.resize(indices32.size());
				for (size_t i = 0; i < indices32.size(); ++i)
				{
					m_indices16[i] = static_cast<uint16_t>(indices32[i]);
				}
			}
			return m_indices16;
		}

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices32;

	private:
		std::vector<uint16_t> m_indices16;
	};

	MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount);

private:
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData);
};