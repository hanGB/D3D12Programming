#include "stdafx.h"
#include "geometry_generator.h"

GeometryGenerator::MeshData GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount)
{

    MeshData meshData;

    // 더미 생성
    // 각 층 높이
    float stackHeight = height / stackCount;

    // 한 층 위의 더미로 올라갈 때의 반지름 변화량 계산
    float radiusStep = (topRadius - bottomRadius) / stackCount;

    uint32_t ringCount = stackCount + 1;

    // 최하단 고리에서 최상단 고리로 올라가며 각 고리의 정점 계산
    for (uint32_t i = 0; i < ringCount; ++i)
    {
        float y = -0.5f * height + i * stackHeight;
        float r = bottomRadius + i * radiusStep;

        // 고리의 정점
        float dTheta = 2.0f * XM_PI / sliceCount;
        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            Vertex vertex;

            float c = cosf(j * dTheta);
            float s = sinf(j * dTheta);

            vertex.position = XMFLOAT3(r * c, y, r * s);
            
            vertex.texCoord.x = (float)j / sliceCount;
            vertex.texCoord.y = 1.0f - (float)i / stackCount;
            
            vertex.tangent = XMFLOAT3(-s, 0.0f, c);

            float dr = bottomRadius - topRadius;
            XMFLOAT3 bitangent(dr * c, -height, dr * s);

            XMVECTOR t = XMLoadFloat3(&vertex.tangent);
            XMVECTOR b = XMLoadFloat3(&bitangent);
            XMVECTOR n = XMVector3Normalize(XMVector3Cross(t, b));
            XMStoreFloat3(&vertex.normal, n);

            meshData.vertices.push_back(vertex);
        }
    }

    //  한 고리의 첫 정점과 마지막 정점은 위치가 같고 텍스처 좌표가 달라 다른 정점
    uint32_t ringVertexCount = sliceCount + 1;

    // 각 더미의 색인 계산
    for (uint32_t i = 0; i < stackCount; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            meshData.indices32.push_back(i * ringVertexCount + j);
            meshData.indices32.push_back((i + 1) * ringVertexCount + j);
            meshData.indices32.push_back((i + 1) * ringVertexCount + j + 1);

            meshData.indices32.push_back(i * ringVertexCount + j);
            meshData.indices32.push_back((i + 1) * ringVertexCount + j + 1);
            meshData.indices32.push_back(i * ringVertexCount + j + 1);
        }
    }

    BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
    BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

    return meshData;
}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData)
{
    uint32_t baseIndex = (uint32_t)meshData.vertices.size();

    float y = 0.5f * height;
    float dTheta = 2.0f * XM_PI / sliceCount;

    // 위 마개 정점은 최상단 고리 정점과 위치가 같지만 텍스처 좌표와 법선이 다름
    for (uint32_t i = 0; i <= sliceCount; ++i)
    {
        float x = topRadius * cosf(i * dTheta);
        float z = topRadius * sinf(i * dTheta);

        // 위 마개의 텍스처 좌표 면적이 밑면에 비례하도록 높이에 비례한 값으로 텍스처 좌표 성분 설정
        float u = x / height * 0.5f;
        float v = z / height * 0.5f;
        
        meshData.vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
    }

    // 마개의 중심 정점
    meshData.vertices.push_back(Vertex(0.0f, y, 0.0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

    // 중심 정점의 색인
    uint32_t centerIndex = (uint32_t)meshData.vertices.size() - 1;

    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        meshData.indices32.push_back(centerIndex);
        meshData.indices32.push_back(baseIndex + i + 1);
        meshData.indices32.push_back(baseIndex + i);
    }
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData)
{
    uint32_t baseIndex = (uint32_t)meshData.vertices.size();

    float y = -0.5f * height;
    float dTheta = 2.0f * XM_PI / sliceCount;

    // 위 마개 정점은 최상단 고리 정점과 위치가 같지만 텍스처 좌표와 법선이 다름
    for (uint32_t i = 0; i <= sliceCount; ++i)
    {
        float x = bottomRadius * cosf(i * dTheta);
        float z = bottomRadius * sinf(i * dTheta);

        // 위 마개의 텍스처 좌표 면적이 밑면에 비례하도록 높이에 비례한 값으로 텍스처 좌표 성분 설정
        float u = x / height * 0.5f;
        float v = z / height * 0.5f;

        meshData.vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
    }

    // 마개의 중심 정점
    meshData.vertices.push_back(Vertex(0.0f, y, 0.0, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

    // 중심 정점의 색인
    uint32_t centerIndex = (uint32_t)meshData.vertices.size() - 1;

    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        meshData.indices32.push_back(centerIndex);
        meshData.indices32.push_back(baseIndex + i);
        meshData.indices32.push_back(baseIndex + i + 1);
    }
}
