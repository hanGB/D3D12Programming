cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProjection;
    float4 gPulseColor;
    float gTime;
};

struct VertexIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};
struct VertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    vout.posH = mul(float4(vin.pos, 1.0f), gWorldViewProjection);
    vout.color = vin.color;
    return vout;
}
