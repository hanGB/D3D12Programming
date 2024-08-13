cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProjection;
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
    
    vin.pos.xy += 0.5f * sin(vin.pos.x) * sin(3.0f * gTime);
    vin.pos.z *= 0.6f + 0.4f * sin(2.0f * gTime);
    
    vout.posH = mul(float4(vin.pos, 1.0f), gWorldViewProjection);
    vout.color = vin.color;
    
    return vout;
}