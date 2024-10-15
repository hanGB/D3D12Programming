
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
    float3 normalL : NORMAL;
};

struct VertexOut
{
    float3 posW : Position;
    float3 normalW : NORMAL;
};

struct GeoOut
{
    float4 posH : SV_Position;
};

// 버텍스 쉐이더
VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
    
    float4 posW = mul(float4(vin.posL, 1.0f), gWorld);
    vout.posW = posW.xyz;
    
    // 비균등 비례가 없다고 가정하여 계산 -> 비균등 비례가 있을 경우 역전치 행렬 사용
    vout.normalW = mul(vin.normalL, (float3x3) gWorld);
    
    return vout;
}

// 지오메트리 쉐이더
[maxvertexcount(2)]
void GS(point VertexOut gin[1],
    uint primitiveID : SV_PrimitiveID,
    inout LineStream<GeoOut> lineStream)
{
    GeoOut gout;
    
    gout.posH = mul(float4(gin[0].posW, 1.0f), gViewProjection);
    lineStream.Append(gout);
    
    float3 pos = gin[0].posW + gin[0].normalW;
    gout.posH = mul(float4(pos, 1.0f), gViewProjection);
    lineStream.Append(gout);
}

[maxvertexcount(2)]
void GSSurface(triangle VertexOut gin[3],
    uint primitiveID : SV_PrimitiveID,
    inout LineStream<GeoOut> lineStream)
{
    float3 pos = gin[0].posW + gin[1].posW + gin[2].posW;
    pos /= 3.0f;
    float3 normal = gin[0].normalW + gin[1].normalW + gin[2].normalW;
    normal = normalize(normal);
    
    GeoOut gout; 
    
    gout.posH = mul(float4(pos, 1.0f), gViewProjection);
    lineStream.Append(gout);
    
    pos += normal;
    gout.posH = mul(float4(pos, 1.0f), gViewProjection);
    lineStream.Append(gout);
}

// 픽셀 쉐이더
float4 PS(GeoOut pin) : SV_Target
{
    return DEBUG_COLOR;
}
