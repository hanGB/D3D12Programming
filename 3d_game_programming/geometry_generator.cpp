#include "stdafx.h"
#include "geometry_generator.h"

GeometryGenerator::MeshData GeometryGenerator::CreateBox(float width, float height, float depth, uint32_t numSubdivisions)
{
    MeshData meshData;

    // 버텍스 생성
    Vertex vertex[24];

    float hW = 0.5f * width;
    float hH = 0.5f * height;
    float hD = 0.5f * depth;

    // 앞쪽
    vertex[0] = Vertex(-hW, -hH, -hD, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertex[1] = Vertex(-hW, +hH, -hD, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertex[2] = Vertex(+hW, +hH, -hD, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertex[3] = Vertex(+hW, -hH, -hD, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // 뒤쪽
    vertex[4] = Vertex(-hW, -hH, +hD, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    vertex[5] = Vertex(+hW, -hH, +hD, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertex[6] = Vertex(+hW, +hH, +hD, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertex[7] = Vertex(-hW, +hH, +hD, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // 위쪽
    vertex[8] = Vertex(-hW, +hH, -hD, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertex[9] = Vertex(-hW, +hH, +hD, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertex[10] = Vertex(+hW, +hH, +hD, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    vertex[11] = Vertex(+hW, +hH, -hD, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // 아래쪽
    vertex[12] = Vertex(-hW, -hH, -hD, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    vertex[13] = Vertex(+hW, -hH, -hD, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    vertex[14] = Vertex(+hW, -hH, +hD, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    vertex[15] = Vertex(-hW, -hH, +hD, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // 왼쪽
    vertex[16] = Vertex(-hW, -hH, +hD, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    vertex[17] = Vertex(-hW, +hH, +hD, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    vertex[18] = Vertex(-hW, +hH, -hD, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    vertex[19] = Vertex(-hW, -hH, -hD, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // 오른쪽
    vertex[20] = Vertex(+hW, -hH, -hD, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    vertex[21] = Vertex(+hW, +hH, -hD, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    vertex[22] = Vertex(+hW, +hH, +hD, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    vertex[23] = Vertex(+hW, -hH, +hD, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

    meshData.vertices.assign(&vertex[0], &vertex[24]);

    // 인덱스 생성
    uint32_t index[36];
    // Fill in the front face index data
    index[0] = 0; index[1] = 1; index[2] = 2;
    index[3] = 0; index[4] = 2; index[5] = 3;

    // Fill in the back face index data
    index[6] = 4; index[7] = 5; index[8] = 6;
    index[9] = 4; index[10] = 6; index[11] = 7;

    // Fill in the top face index data
    index[12] = 8; index[13] = 9; index[14] = 10;
    index[15] = 8; index[16] = 10; index[17] = 11;

    // Fill in the bottom face index data
    index[18] = 12; index[19] = 13; index[20] = 14;
    index[21] = 12; index[22] = 14; index[23] = 15;

    // Fill in the left face index data
    index[24] = 16; index[25] = 17; index[26] = 18;
    index[27] = 16; index[28] = 18; index[29] = 19;

    // Fill in the right face index data
    index[30] = 20; index[31] = 21; index[32] = 22;
    index[33] = 20; index[34] = 22; index[35] = 23;

    meshData.indices32.assign(&index[0], &index[36]);

    // 세분 횟수에 상한 설정
    numSubdivisions = std::min<uint32_t>(numSubdivisions, 6u);

    for (uint32_t i = 0; i < numSubdivisions; ++i)
    {
        Subdivide(meshData);
    }

    return meshData;
}

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

GeometryGenerator::MeshData GeometryGenerator::CreateGeosphere(float radius, uint32_t numSubdivisions)
{
    MeshData meshData;

    // 세분 횟수에 상한 설정
    numSubdivisions = std::min<uint32_t>(numSubdivisions, 6u);

    // 정이십면체를 테셀레이션해서 구를 근사함
    const float x = 0.525731f;
    const float z = 0.850651f;

    XMFLOAT3 pos[12] =
    {
        XMFLOAT3(-x, 0.0f, z), XMFLOAT3(x, 0.0f, z),
        XMFLOAT3(-x, 0.0f, -z), XMFLOAT3(x, 0.0f, -z),
        XMFLOAT3(0.0f, z, x), XMFLOAT3(0.0f, z, -x),
        XMFLOAT3(0.0f, -z, x), XMFLOAT3(0.0f, -z, -x),
        XMFLOAT3(z, x, 0.0f), XMFLOAT3(-z, x, 0.0f),
        XMFLOAT3(z, -x, 0.0f), XMFLOAT3(-z, -x, 0.0f)
    };

    uint32_t k[60] =
    {
        1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4,
        1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2,
        3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
        10,1,6, 11,0,9, 2,11,9, 5,2,9, 11,2,7
    };

    meshData.vertices.resize(12);
    meshData.indices32.assign(&k[0], &k[60]);

    for (uint32_t i = 0; i < 12; ++i)
    {
        meshData.vertices[i].position = pos[i];
    }
    for (uint32_t i = 0; i < numSubdivisions; ++i)
    {
        Subdivide(meshData);
    }

    // 정점들을 구에 투명 후 비례
    for (uint32_t i = 0; i < meshData.vertices.size(); ++i)
    {
        // 단위 구에 투영
        XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.vertices[i].position));

        // 주어진 반지름으로 비례(원래의 구에 투영)
        XMVECTOR p = radius * n;

        XMStoreFloat3(&meshData.vertices[i].position, p);
        XMStoreFloat3(&meshData.vertices[i].normal, n);

        // 구면 좌표로부터 텍스처 좌표 계산
        float theta = atan2f(meshData.vertices[i].position.z, meshData.vertices[i].position.x);

        // 각도 세타를 [0, 2pi]로 한정
        if (theta < 0.0f)
        {
            theta += XM_2PI;
        }
        float phi = acosf(meshData.vertices[i].position.y / radius);

        meshData.vertices[i].texCoord.x = theta / XM_PI;
        meshData.vertices[i].texCoord.y = phi / XM_PI;

        // 세타에 대한 p의 편미분 계수 계산
        meshData.vertices[i].tangent.x = -radius * sinf(phi) * sinf(theta);
        meshData.vertices[i].tangent.y = 0.0f;
        meshData.vertices[i].tangent.z = radius * sinf(phi) * cosf(theta);

        XMVECTOR t = XMLoadFloat3(&meshData.vertices[i].tangent);
        XMStoreFloat3(&meshData.vertices[i].tangent, XMVector3Normalize(t));
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
        float z = halfDepth - i * dz;

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

GeometryGenerator::MeshData GeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
{
    MeshData meshData;

    meshData.vertices.resize(4);
    meshData.indices32.resize(6);

    // 버텍스 설정
    meshData.vertices[0] = Vertex(
        x, y - h, depth,
        0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f);

    meshData.vertices[1] = Vertex(
        x, y, depth,
        0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f);

    meshData.vertices[2] = Vertex(
        x + w, y, depth,
        0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f);

    meshData.vertices[3] = Vertex(
        x + w, y - h, depth,
        0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f);

    // 인덱스 설정
    meshData.indices32[0] = 0;
    meshData.indices32[1] = 1;
    meshData.indices32[2] = 2;

    meshData.indices32[3] = 0;
    meshData.indices32[4] = 2;
    meshData.indices32[5] = 3;

    return meshData;
}

void GeometryGenerator::Subdivide(MeshData& meshData)
{
    // 복사본을 만든 후 데이터 초기화
    MeshData inputCopy = meshData;
    meshData.vertices.resize(0);
    meshData.indices32.resize(0);

    //     v1
    //    /  \
    //   m0 ─ m1
    //  /  \ /  \
    // v0 ─ m2 ─ v2

    uint32_t numTris = (uint32_t)inputCopy.indices32.size() / 3;
    for (uint32_t i = 0; i < numTris; ++i)
    {
        Vertex v0 = inputCopy.vertices[inputCopy.indices32[i * 3 + 0]];
        Vertex v1 = inputCopy.vertices[inputCopy.indices32[i * 3 + 1]];
        Vertex v2 = inputCopy.vertices[inputCopy.indices32[i * 3 + 2]];

        // 중간 지점 생성
        Vertex m0 = MidPoint(v0, v1);
        Vertex m1 = MidPoint(v1, v2);
        Vertex m2 = MidPoint(v0, v2);

        // 추가
        meshData.vertices.push_back(v0);
        meshData.vertices.push_back(v1);
        meshData.vertices.push_back(v2);
        meshData.vertices.push_back(m0);
        meshData.vertices.push_back(m1);
        meshData.vertices.push_back(m2);

        meshData.indices32.push_back(i * 6 + 0);
        meshData.indices32.push_back(i * 6 + 3);
        meshData.indices32.push_back(i * 6 + 5);

        meshData.indices32.push_back(i * 6 + 3);
        meshData.indices32.push_back(i * 6 + 4);
        meshData.indices32.push_back(i * 6 + 5);

        meshData.indices32.push_back(i * 6 + 5);
        meshData.indices32.push_back(i * 6 + 4);
        meshData.indices32.push_back(i * 6 + 2);

        meshData.indices32.push_back(i * 6 + 3);
        meshData.indices32.push_back(i * 6 + 1);
        meshData.indices32.push_back(i * 6 + 4);
    }
}

GeometryGenerator::Vertex GeometryGenerator::MidPoint(const Vertex& v0, const Vertex& v1)
{
    XMVECTOR p0 = XMLoadFloat3(&v0.position);
    XMVECTOR p1 = XMLoadFloat3(&v1.position);

    XMVECTOR n0 = XMLoadFloat3(&v0.normal);
    XMVECTOR n1 = XMLoadFloat3(&v1.normal);

    XMVECTOR tan0 = XMLoadFloat3(&v0.tangent);
    XMVECTOR tan1 = XMLoadFloat3(&v1.tangent);

    XMVECTOR tex0 = XMLoadFloat2(&v0.texCoord);
    XMVECTOR tex1 = XMLoadFloat2(&v1.texCoord);

    // 중간 지점 계산
    XMVECTOR pos = 0.5f * (p0 + p1);
    XMVECTOR normal = XMVector3Normalize(0.5f * (n0 + n1));
    XMVECTOR tangent = XMVector3Normalize(0.5f * (tan0 + tan1));
    XMVECTOR texCoord = 0.5f * (tex0 + tex1);

    Vertex vertex;
    XMStoreFloat3(&vertex.position, pos);
    XMStoreFloat3(&vertex.normal, normal);
    XMStoreFloat3(&vertex.tangent, tangent);
    XMStoreFloat2(&vertex.texCoord, texCoord);

    return vertex;
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