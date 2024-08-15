cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProjection;
    float4 gPulseColor;
    float gTime;
};

struct VertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};

float4 main(VertexOut pin) : SV_Target
{
    const float pi = 3.141592;
    
    // 사인 함수로 [0, 1] 구간에서 진동하는 값 계산
    float s = 0.5f * sin(2.0f * gTime - 0.25f * pi) + 0.5f;

    float4 c = lerp(pin.color, gPulseColor, s);
    
    return c;
}
