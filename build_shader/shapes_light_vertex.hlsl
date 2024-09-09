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
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_Position;
    float3 normalW : NORMAL;
    float4 color : COLOR;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    float4 posW = mul(float4(vin.posL, 1.0f), gWorld);
    vout.posH = mul(posW, gViewProjection);
    vout.color = vin.color;
    
    // 비균등 비례가 없다고 가정하여 계산 -> 비균등 비례가 있을 경우 역전치 행렬 사용
    vout.normalW = mul(vin.normalL, (float3x3) gWorld);
    
    return vout;
}