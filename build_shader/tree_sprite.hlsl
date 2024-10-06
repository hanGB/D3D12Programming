
#include "general_setting.hlsli"
#include "lighting_utils.hlsli"

// 텍스처
Texture2DArray gTreeMapArray : register(t0);

// 샘플러
SamplerState gSamPointWrap          : register(s0);
SamplerState gSamPointClamp         : register(s1);
SamplerState gSamLinearWrap         : register(s2);
SamplerState gSamLinearClamp        : register(s3);
SamplerState gSamAnisotropicWrap    : register(s4);
SamplerState gSamAnisotropicClamp   : register(s5);

// 패스 상수
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInView;
    float4x4 gProjection;
    float4x4 gInvProjection;
    float4x4 gViewProjection;
    float4x4 gInvViewProjection;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    float2 temp; // float4를 맞추기 위한 변수
    
    Light gLights[MAX_LIGHTS];
};

// 머터리얼 상수
cbuffer cbMaterial : register(b2)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;
    
    float4x4 gMatTransform;
};

// 인풋 / 아웃풋
struct VertexIn
{
    float3 posW     : POSITION;
    float2 sizeW    : SIZE;
};

struct VertexOut
{
    float3 centerW  : POSITION;
    float2 sizeW    : SIZE;
};

struct GeoOut
{
    float4 posH         : SV_Position;
    float3 posW         : POSITION;
    float3 normalW      : NORMAL;
    float2 texCoord     : TEXCOORD;
    uint   primitiveID  : SV_PrimitiveID;
};

// 버텍스 쉐이더
VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // 그대로 넘김
    vout.centerW = vin.posW;
    vout.sizeW = vin.sizeW;

    return vout;
}

// 지오메트리 쉐이더
// 각 점을 사각형으로 확장하므로 최대 출력 4개
[maxvertexcount(4)]
void GS(point VertexOut gin[1],
    uint primitiveID : SV_PrimitiveID,
    inout TriangleStream<GeoOut> triangleStream)
{
    // xz 평면에 수직인 빌보드 생성
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].centerW;
    look.y = 0.0f;
    look = normalize(look);
    float3 right = cross(up, look);
    
    // 월드 스페이스 기준의 삼각형 띠 버텍스 계산
    float halfWidth = 0.5f * gin[0].sizeW.x;
    float halfHeight = 0.5f * gin[0].sizeW.y;
    
    float4 v[4];
    v[0] = float4(gin[0].centerW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].centerW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].centerW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].centerW - halfWidth * right + halfHeight * up, 1.0f);
    
    // 사각형 버테스를 동차 절단 공간으로 변환하고 출력
    float2 texCoord[4] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, 0.0f)
    };
    
    GeoOut gout;
    [unroll] // 실제로 반복하는 대신 별도로 처리해 성능 최적화
    for (int i = 0; i < 4; ++i)
    {
        gout.posH = mul(v[i], gViewProjection);
        gout.posW = v[i].xyz;
        gout.normalW = look;
        gout.texCoord = texCoord[i];
        gout.primitiveID = primitiveID;
        
        triangleStream.Append(gout);
    }
}

// 픽셀 쉐이더
float4 PS(GeoOut pin) : SV_Target
{
    float3 uvw = float3(pin.texCoord, pin.primitiveID % 3);
    
    float4 diffuseAlbedo = gTreeMapArray.Sample(gSamAnisotropicWrap, uvw) * gDiffuseAlbedo;
    
#ifdef ALPHA_TEST
    // 알파값이 0.1 이하면 픽셀 잘라내고 종료
    clip(diffuseAlbedo.a - 0.1f);
#endif    

    // 노멀 다시 정규화
    pin.normalW = normalize(pin.normalW);
    
    // 조명이 되는 점에서 눈으로의 벡터
    float3 toEye = gEyePosW - pin.posW;
    float distanceToEye = length(toEye);
    toEye /= distanceToEye;
    
    // 직접 조명
    const float shininess = 1.0f - gRoughness;
    Material material = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, material, pin.posW, pin.normalW, toEye, shadowFactor);
    
    float4 litColor = (gAmbientLight + directLight) * diffuseAlbedo;
  
#ifdef FOG
    // 안개
    float fogAmount = saturate((distanceToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
#endif
    
    // 흔히 하는 방식대로 분산 재질에서 알파를 가져옴
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}
