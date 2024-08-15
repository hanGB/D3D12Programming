struct VertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};

float4 main(VertexOut pin) : SV_Target
{
    return pin.color;
}
