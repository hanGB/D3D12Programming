cbuffer CameraInfo : register(b1)
{
    matrix view : packoffset(c0);
    matrix projection : packoffset(c4);
}

// 버텍스 셰이더 입력을 위한 구조체
struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float4x4 world : WORLDMATRIX;
    float4 instanceColor : INSTANCECOLOR;
};

// 버텍스 쉐이더 출력(픽셀 셰이더 입력)을 위한 구조체
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.position = mul(mul(mul(float4(input.position, 1.0f), input.world), view), projection);
    output.color = input.color + input.instanceColor;
    
    return output;
}
