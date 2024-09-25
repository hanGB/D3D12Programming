
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
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
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
    float3 posL     : POSITION;
    float2 uv       : TEXCOORD;
    float3 normalL  : NORMAL;
};

struct VertexOut
{
    float4 posH     : SV_Position;
    float3 posW     : Position;
    float2 uv       : TEXCOORD;
    float3 normalW  : NORMAL;
};

VertexOut main(VertexIn vin)
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