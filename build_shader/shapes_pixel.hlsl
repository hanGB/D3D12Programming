struct VertexOut
{
    float4 posH : SV_Position;
    float4 color : COLOR;
};

float4 main(VertexOut pin) : SV_TARGET
{
    return pin.color;
}