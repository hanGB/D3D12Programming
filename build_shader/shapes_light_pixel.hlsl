struct VertexOut
{
    float4 posH : SV_Position;
    float3 normalW : NORMAL;
    float4 color : COLOR;
};

float4 main(VertexOut pin) : SV_TARGET
{
    return float4(pin.normalW, 1.0f);
}