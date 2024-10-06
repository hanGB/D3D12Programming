
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
    float2 uv : TEXCOORD;
    float3 normalL : NORMAL;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float3 posW : Position;
    float2 uv : TEXCOORD;
    float3 normalW : NORMAL;
};

// 버텍스 쉐이더
VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
    
    float4 posW = mul(float4(vin.posL, 1.0f), gWorld);
    vout.posW = posW.xyz;
    
    vout.posH = mul(posW, gViewProjection);
    
    float4 uv = mul(float4(vin.uv, 0.0f, 1.0f), gTexTransform);
    vout.uv = mul(uv, gMatTransform).xy;
    
    // 비균등 비례가 없다고 가정하여 계산 -> 비균등 비례가 있을 경우 역전치 행렬 사용
    vout.normalW = mul(vin.normalL, (float3x3) gWorld);

    return vout;
}

// 픽셀 쉐이더
float4 FS(VertexOut pin) : SV_TARGET
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
    
    float4 litColor = (ambient + directLight) * gDiffuseMap.Sample(gSamAnisotropicWrap, pin.uv);

#ifdef FOG    
    // 안개
    float fogAmount = saturate((distanceToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
#endif
    
    // 흔히 하는 방식대로 분산 재질에서 알파를 가져옴
    litColor.a = gDiffuseAlbedo.a;
    
    return litColor;
}

// 픽셀 쉐이더(알파 테스트)
float4 AlphaTestedFS(VertexOut pin) : SV_Target
{   
    float4 diffuseAlbedo = gDiffuseMap.Sample(gSamAnisotropicWrap, pin.uv) * gDiffuseAlbedo;
    
#ifdef ALPHA_TEST
    // 알파값이 0.1 이하면 픽셀 잘라내고 종료
    clip(diffuseAlbedo.a - 0.1f);
#endif   
    
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
    
    float4 litColor = (ambient + directLight) * diffuseAlbedo;
    
#ifdef FOG
    // 안개
    float fogAmount = saturate((distanceToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
#endif
    
    // 흔히 하는 방식대로 분산 재질에서 알파를 가져옴
    litColor.a = gDiffuseAlbedo.a;
    
    return litColor;
}
