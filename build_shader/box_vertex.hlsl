cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProjection;
    float gTime;
    float gAnimationTime;
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

float easeInOutQuint(float x);

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    vin.pos.xy += 0.5f * sin(vin.pos.x) * sin(3.0f * gTime);
    vin.pos.z *= 0.6f + 0.4f * sin(2.0f * gTime);
    
    vout.posH = mul(float4(vin.pos, 1.0f), gWorldViewProjection);
    vout.color = float4(vin.color.rgb * easeInOutQuint(gAnimationTime), vin.color.a);
    return vout;
}

float easeInOutQuint(float x)
{
    return x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
}