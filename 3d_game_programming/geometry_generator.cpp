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

GeometryGenerator::MeshData GeometryGenerator::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
    MeshData meshData;

    // 꼭대기, 바닥 정점
    Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    meshData.vertices.push_back(topVertex);

    // 더미 생성
    // 각 스텝
    float phiStep = XM_PI / stackCount;
    float thetaStep = 2.0f * XM_PI / sliceCount;

    // 꼭대기 정점 바로 밑 원에서 바닥 정점 바로 위 원으로 내려가며 각 원의 정점 계산
    for (uint32_t i = 0; i < stackCount - 1; ++i)
    {
        float phi = i * phiStep;
        
        // 고리의 정점
        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            Vertex vertex;

            // 구의 매개변수 방정식으로 정점 위치 계산
            vertex.position.x = radius * sinf(phi) * cosf(theta);
            vertex.position.z = radius * sinf(phi) * sinf(theta);
            vertex.position.y = radius * cosf(phi);

            // 세타에 대한 편도 함수
            vertex.tangent.x = -radius * sinf(phi) * sinf(theta);
            vertex.tangent.z = +radius * sinf(phi) * cosf(theta);
            vertex.tangent.y = 0.0f;

            // 탄젠트 정규화
            XMVECTOR t = XMLoadFloat3(&vertex.tangent);
            XMStoreFloat3(&vertex.tangent, XMVector3Normalize(t));
            
            // 구는 정규화된 위치가 노멀 벡터임
            XMVECTOR p = XMLoadFloat3(&vertex.position);
            XMStoreFloat3(&vertex.normal, XMVector3Normalize(p));

            vertex.texCoord.x = theta / XM_2PI;
            vertex.texCoord.y = phi / XM_PI;

            meshData.vertices.push_back(vertex);
        }
    }

    meshData.vertices.push_back(bottomVertex);

    // 각 더미의 색인 계산
    
    // 꼭대기 정점과 바로 밑 원 사이의 삼각형 계산
    for (uint32_t i = 1; i <= sliceCount; ++i)
    {
        meshData.indices32.push_back(0);
        meshData.indices32.push_back(i + 1);
        meshData.indices32.push_back(i);
    }

    // 꼭대기 정점과 바닥 정점을 포함하지 않는 안쪽 정점 사이의 삼각형 계산
    uint32_t baseIndex = 1;
    uint32_t ringVertexCount = sliceCount + 1;
    for (uint32_t i = 0; i < stackCount - 2; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            meshData.indices32.push_back(baseIndex + i * ringVertexCount + j);
            meshData.indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            meshData.indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            meshData.indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }


    // 바닥 정점 인덱스 계산(가장 마지막 정점)
    uint32_t bottomVertexIndex = (uint32_t)meshData.vertices.size() - 1;

    // 바닥 정점과 바로 위 원 사이의 삼각형 계산
    for (uint32_t i = 0; i < sliceCount; ++i)
    {
        meshData.indices32.push_back(bottomVertexIndex);
        meshData.indices32.push_back(bottomVertexIndex - ringVertexCount + i);
        meshData.indices32.push_back(bottomVertexIndex - ringVertexCount + i + 1);
    }

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateGrid(float width, float depth, uint32_t m, uint32_t n)
{
    MeshData meshData;

    uint32_t vertexCount = m * n;
    uint32_t faceCount = (m - 1) * (n - 1) * 2;

    // 버텍스 계산
    float halfWidth = 0.5f * width;
    float halfDepth = 0.5f * depth;

    float dx = width / (n - 1);
    float dz = depth / (m - 1);

    float du = 1.0f / (n - 1);
    float dv = 1.0f / (m - 1);

    meshData.vertices.resize(vertexCount);

    for (uint32_t i = 0, k = 0; i < m; ++i)
    {
        float z = -halfDepth + i * dz;

        for (uint32_t j = 0; j < n; ++j, ++k)
        {
            float x = -halfWidth + j * dx;

            meshData.vertices[k].position = XMFLOAT3(x, 0.0f, z);
            meshData.vertices[k].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
            meshData.vertices[k].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

            meshData.vertices[k].texCoord.x = i * du;
            meshData.vertices[k].texCoord.x = j * dv;
        }
    }

    // 인덱스 계산
    meshData.indices32.resize(faceCount * 3);

    for (uint32_t i = 0, k = 0; i < m - 1; ++i)
    {
        for (uint32_t j = 0; j < n - 1; ++j)
        {
            meshData.indices32[k++] = i * n + j;
            meshData.indices32[k++] = i * n + j + 1;
            meshData.indices32[k++] = (i + 1) * n + j;
            meshData.indices32[k++] = (i + 1) * n + j;
            meshData.indices32[k++] = i * n + j + 1;
            meshData.indices32[k++] = (i + 1) * n + j + 1;
        }
    }

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

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, GeometryGenerator::MeshData& meshData)
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