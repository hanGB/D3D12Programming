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

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

struct VertexIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float4 color : COLOR;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    float4 posW = mul(float4(vin.pos, 1.0f), gWorld);
    vout.posH = mul(posW, gProjection);
    vout.color = vin.color;
    
    return vout;
}