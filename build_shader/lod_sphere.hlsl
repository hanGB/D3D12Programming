
#include "general_setting.hlsli"
#include "lighting_utils.hlsli"

// 텍스처
Texture2D gDiffuseMap : register(t0);

// 샘플러
SamplerState gSamPointWrap : register(s0);
SamplerState gSamPointClamp : register(s1);
SamplerState gSamLinearWrap : register(s2);
SamplerState gSamLinearClamp : register(s3);
SamplerState gSamAnisotropicWrap : register(s4);
SamplerState gSamAnisotropicClamp : register(s5);

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInView;
    float4x4 gProjection;
    float4x4 gInvProjection;
    float4x4 gViewProjection;
    float4x4 gInvViewProjection;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    float2 temp; // float4를 맞추기 위한 변수
    
    Light gLights[MAX_LIGHTS];
};

cbuffer cbMaterial : register(b2)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;

    float4x4 gMatTransform;
};

struct VertexIn
{
    float3 posL : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normalL : NORMAL;
};

struct VertexOut
{
    float3 posL : Position;
    float2 texCoord : TEXCOORD;
    float3 normalL : NORMAL;
};

struct GeoOut
{
    float4 posH : SV_Position;
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 texCoord : TEXCOORD;
};

// 버텍스 쉐이더
VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;
    
    vout.posL = vin.posL;
    vout.normalL = vin.normalL;
    vout.texCoord = vin.texCoord;;
    
    return vout;
}

VertexOut MidPoint(VertexOut v0, VertexOut v1)
{
    // 중간 지점 계산
    VertexOut mid;
    mid.posL = 0.5f * (v0.posL + v1.posL);
    mid.normalL = normalize(0.5f * (v0.normalL + v1.normalL));
    mid.texCoord = 0.5f * (v0.texCoord + v1.texCoord);

    return mid;
}

void Subdivide(VertexOut inVertexs[3], out VertexOut outVertexs[6])
{
    outVertexs[0] = inVertexs[0];
    outVertexs[1] = MidPoint(inVertexs[0], inVertexs[1]);
    outVertexs[2] = MidPoint(inVertexs[2], inVertexs[0]);
    outVertexs[3] = MidPoint(inVertexs[1], inVertexs[2]);
    outVertexs[4] = inVertexs[2];
    outVertexs[5] = inVertexs[1];
}

void ProjectionToSphere(float radius, inout VertexOut outVertexs[6])
{
    // 정점들을 구에 투명 후 비례
    for (int i = 0; i < 6; ++i)
    {
        // 단위 구에 투영
        float3 n = normalize(outVertexs[i].posL);

        // 주어진 반지름으로 비례(원래의 구에 투영)
        float3 p = radius * n;

        outVertexs[i].posL = p;
        outVertexs[i].normalL = n;
        
        // 구면 좌표로부터 텍스처 좌표 계산
        float theta = atan2(outVertexs[i].posL.z, outVertexs[i].posL.x);

        // 각도 세타를 [0, 2pi]로 한정
        if (theta < 0.0f)
        {
            theta += 2.0f * PI_VALUE;
        }
        float phi = acos(outVertexs[i].posL.y / PI_VALUE);

        outVertexs[i].texCoord.x = theta / PI_VALUE;
        outVertexs[i].texCoord.y = phi / PI_VALUE;
    }
}

void OutputSubdivision(VertexOut outVertexs[6], inout TriangleStream<GeoOut> triangleStream)
{
    GeoOut gout[6];
    
    [unroll] // 실제로 반복하는 대신 별도로 처리해 성능 최적화
    for (int i = 0; i < 6; ++i)
    {
        gout[i].posW = mul(float4(outVertexs[i].posL, 1.0f), gWorld).xyz;
        gout[i].normalW = mul(outVertexs[i].normalL, (float3x3) gWorld);
        
        float4 texCoord = mul(float4(outVertexs[i].texCoord, 0.0f, 1.0f), gTexTransform);
        gout[i].texCoord = mul(texCoord, gMatTransform).xy;
        
        gout[i].posH = mul(float4(gout[i].posW, 1.0f), gViewProjection);
    }
    
    [unroll]
    for (int j = 0; j < 5; ++j)
    {
        triangleStream.Append(gout[j]);
    }
    triangleStream.RestartStrip();
    
    triangleStream.Append(gout[1]);
    triangleStream.Append(gout[5]);
    triangleStream.Append(gout[3]);
}

// 지오메트리 쉐이더
[maxvertexcount(8)]
void GS(triangle VertexOut gin[3],
    uint primitiveID : SV_PrimitiveID,
    inout TriangleStream<GeoOut> triangleStream)
{
    VertexOut outVertexs[6];
    Subdivide(gin, outVertexs);
    ProjectionToSphere(length(gin[0].posL), outVertexs);
    OutputSubdivision(outVertexs, triangleStream);
}

// 픽셀 쉐이더
float4 PS(GeoOut pin) : SV_TARGET
{
    // 노멀 다시 정규화
    pin.normalW = normalize(pin.normalW);
    
    // 조명이 되는 점에서 눈으로의 벡터
    float3 toEye = gEyePosW - pin.posW;
    float distanceToEye = length(toEye);
    toEye = normalize(toEye);
    
    // 간접 조명을 흉내 내는 주변광
    float4 ambient = gAmbientLight * gDiffuseAlbedo;

    // 직접 조명
    const float shininess = 1.0f - gRoughness;
    Material material = { gDiffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, material, pin.posW, pin.normalW, toEye, shadowFactor);
    
    float4 litColor = (ambient + directLight) * gDiffuseMap.Sample(gSamAnisotropicWrap, pin.texCoord);

#ifdef FOG    
    // 안개
    float fogAmount = saturate((distanceToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
#endif
    
    // 흔히 하는 방식대로 분산 재질에서 알파를 가져옴
    litColor.a = gDiffuseAlbedo.a;
    
    return litColor;
}

