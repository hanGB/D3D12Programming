cbuffer cbPass : register(b2)
{
    float4x4 gView;
    float4x4 gInView;
    float4x4 gProjection;
    float4x4 gInvProjection;
    float4x4 gViewProjection;
    float4x4 gInvViewProjection;
    float3 gEyeposW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
};

cbuffer cbMaterial : register(b1)
{
    float4 diffuseAlbedo;
    float3 fresnelR0;
    float roughness;

    float4x4 matTransform;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float4 color : COLOR;
};

#include "lighting_utils.hlsli"

float4 main(VertexOut pin) : SV_TARGET
{
    float att = CalculateAttenuation(50.0f, 1.0f, 100.0f);
    
    return diffuseAlbedo * att;
}