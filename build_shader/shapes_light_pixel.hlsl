cbuffer cbPass : register(b1)
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

cbuffer cbMaterial : register(b2)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;

    float4x4 gMatTransform;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float3 normalW : NORMAL;
    float4 color : COLOR;
};

float4 main(VertexOut pin) : SV_TARGET
{
    return gDiffuseAlbedo;
}