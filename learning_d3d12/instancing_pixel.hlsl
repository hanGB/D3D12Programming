// 버텍스 쉐이더 출력(픽셀 셰이더 입력)을 위한 구조체
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return input.color;
}