struct Light
{
    float3  strength;
    float   falloffStart;
    float3  direction;
    float   falloffEnd;
    float3  position;
    float   spotPower;
};

struct Material
{
    float4 diffuseAlbedo;
    float3 frensnelR0;
    
    float shininess;
};

// 선형 감쇄
float CalculateAttenuation(float distance, float falloffStart, float falloffEnd);
// 슐릭 근사(프레넬 반사의 근사값 제공)
float3 SchlickFresnel(float3 r0, float3 normal, float3 lightVector);
// 람베르트 코사인 법칙에 따른 빛의 세기 변경
float3 CaculateLightStrengthWithCosineEmissionLaw(float3 baseStrenght, float3 normal, float3 lightVector);
// 블린 퐁
float3 BlinnPhong(float3 lightStrength, float3 lightVector, float3 normal, float3 toEye, Material material);
// 디렉셔널 라이트
float3 ComputeDirectionalLight(Light light, Material material, float3 normal, float3 toEye);
// 포인트 라이트
float3 ComputePointLight(Light light, Material material, float3 pos, float3 normal, float3 toEye);
// 스포트 라이트
float3 ComputeSpotLight(Light light, Material material, float3 pos, float3 normal, float3 toEye);



float CalculateAttenuation(float distance, float falloffStart, float falloffEnd)
{
    // saturate : [0, 1]사이로 변경 해주는 함수
    return saturate((falloffEnd - distance) / (falloffEnd - falloffStart));
}

float3 SchlickFresnel(float3 r0, float3 normal, float3 lightVector)
{
    float cosIncidentAngle = saturate(dot(normal, lightVector));
    
    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = r0 + (1.0f - r0) * (f0 * f0 * f0 * f0 * f0);
    
    return reflectPercent;
}

float3 CaculateLightStrengthWithCosineEmissionLaw(float3 baseStrenght, float3 normal, float3 lightVector)
{
    // 람베르트 코사인 법칙에 따른 빛의 세기 변경
    float ndotl = max(dot(lightVector, normal), 0.0f);
   
    return baseStrenght * ndotl;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVector, float3 normal, float3 toEye, Material material)
{
    // 거칠기에서 얻은 광택도로부터 m 유도
    const float m = material.shininess * 256.0f;
    float3 halfVector = normalize(toEye + lightVector);
    
    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVector, normal), 0.0f), m) / 8.0f;
    float3 fresnel = SchlickFresnel(material.frensnelR0, halfVector, lightVector);
    
    float3 specularAlbedo = roughnessFactor * fresnel;
    
    // [0, 1] 사이로 변경(LDR)
    specularAlbedo = specularAlbedo / (specularAlbedo + 1.0f);
    
    return (material.diffuseAlbedo.rgb + specularAlbedo) * lightStrength;
}

float3 ComputeDirectionalLight(Light light, Material material, float3 normal, float3 toEye)
{
    // 빛 벡터: 광선들이 나아가는 방향의 반대
    float3 lightVector = -light.direction;
    
    // 빛의 세기 변경
    float3 lightStrength = CaculateLightStrengthWithCosineEmissionLaw(light.strength, normal, lightVector);
    
    return BlinnPhong(lightStrength, lightVector, normal, toEye, material);
}

float3 ComputePointLight(Light light, Material material, float3 pos, float3 normal, float3 toEye)
{
    // 표면에서 광원으로의 벡터
    float3 lightVector = light.position - pos;
    
    // 광원과 표면 사이의 거리
    float distance = length(lightVector);
    
    // 범위 판정
    if (distance > light.falloffEnd)
    {
        return 0.0f;
    }
    
    // 빛 벡터 정규화
    lightVector /= distance;
    
    // 빛의 세기 변경
    float3 lightStrength = CaculateLightStrengthWithCosineEmissionLaw(light.strength, normal, lightVector);
    
    // 거리에 따른 빛 감쇄
    float att = CalculateAttenuation(distance, light.falloffStart, light.falloffEnd);
    lightStrength *= att;
    
    return BlinnPhong(lightStrength, lightVector, normal, toEye, material);
}

float3 ComputeSpotLight(Light light, Material material, float3 pos, float3 normal, float3 toEye)
{
    // 표면에서 광원으로의 벡터
    float3 lightVector = light.position - pos;
    
    // 광원과 표면 사이의 거리
    float distance = length(lightVector);
    
    // 범위 판정
    if (distance > light.falloffEnd)
    {
        return 0.0f;
    }
    
    // 빛 벡터 정규화
    lightVector /= distance;
    
    // 빛의 세기 변경
    float3 lightStrength = CaculateLightStrengthWithCosineEmissionLaw(light.strength, normal, lightVector);
    
    // 거리에 따른 빛 감쇄
    float att = CalculateAttenuation(distance, light.falloffStart, light.falloffEnd);
    lightStrength *= att;
    
    // 방향과 스포트 라이트 계수로 세기 변경
    float spotFactor = pow(max(dot(-lightVector, light.direction), 0.0f), light.spotPower);
    lightStrength *= spotFactor;
    
    return BlinnPhong(lightStrength, lightVector, normal, toEye, material);
}