#include "lighting_utils.hlsli"

#define MAX_LIGHTS 16

#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

cbuffer cbPass : register(b2)
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
    
    Light gLights[MAX_LIGHTS];
};

cbuffer cbMaterial : register(b1)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;

    float4x4 gMatTransform;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float3 posW : Position;
    float3 normalW : NORMAL;
};

float4 ComputeLighting(Material material, float3 pos, float3 normal, float3 toEye, float3 shadowFactor);

float4 main(VertexOut pin) : SV_TARGET
{
    // 노멀 다시 정규화
    pin.normalW = normalize(pin.normalW);
    
    // 조명이 되는 점에서 눈으로의 벡터
    float3 toEye = normalize(gEyePosW - pin.posW);
    
    // 간접 조명을 흉내 내는 주변광
    float ambient = gAmbientLight * gDiffuseAlbedo;

    // 직접 ㅈ명
    const float shininess = 1.0f - gRoughness;
    Material material = { gDiffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(material, pin.posW, pin.normalW, toEye, shadowFactor);
    
    float4 litColor = ambient + directLight;
    
    // 흔히 하는 방식대로 분산 재질에서 알파를 가져옴
    litColor.a = gDiffuseAlbedo.a;
    
    return litColor;
}

float4 ComputeLighting(Material material, float3 pos, float3 normal, float3 toEye, float3 shadowFactor)
{
    float3 result = 0.0f;
    
    int i = 0;
    
#if (NUM_DIR_LIGHTS > 0)
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeDirectionalLight(gLights[i], material, normal, toEye);
    }
#endif
    
#if (NUM_POINT_LIGHTS > 0)
    for (i; i < NUM_POINT_LIGHTS; ++i)
    {
        result += ComputePointLight(gLights[i], material, pos, normal, toEye);
    }
#endif
    
#if (NUM_SPOT_LIGHTS > 0)
    for (i; i < NUM_SPOT_LIGHTS; ++i)
    {
        result += ComputeSpotLight(gLights[i], material, pos, normal, toEye);
    }
#endif
    
    return float4(result, 0.0f);
}