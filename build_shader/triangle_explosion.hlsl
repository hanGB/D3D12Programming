
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
    VertexOut vout = (VertexOut) 0.0f;
    
    vout.posL = vin.posL;
    vout.normalL = vin.normalL;
    vout.texCoord = vin.texCoord;;
    
    return vout;
}

// 지오메트리 쉐이더
[maxvertexcount(3)]
void GS(triangle VertexOut gin[3],
    uint primitiveID : SV_PrimitiveID,
    inout TriangleStream<GeoOut> triangleStream)
{
    // 면 법선 계산
    float3 surfaceNormal = gin[0].normalL + gin[1].normalL + gin[2].normalL;
    surfaceNormal = normalize(surfaceNormal);
    float speed = 0.1f;
    
    GeoOut gout;
    
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        gout.posW = mul(float4(gin[i].posL, 1.0f), gWorld).xyz;
        gout.normalW = mul(gin[i].normalL, (float3x3) gWorld);
        
        gout.posW += surfaceNormal * speed * gTotalTime;
        
        float4 texCoord = mul(float4(gin[i].texCoord, 0.0f, 1.0f), gTexTransform);
        gout.texCoord = mul(texCoord, gMatTransform).xy;
        
        gout.posH = mul(float4(gout.posW, 1.0f), gViewProjection);
        
        triangleStream.Append(gout);
    }
       
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

