#pragma once
#include "d3d_util.h"
#include "upload_buffer.h"
#include "math_helper.h"
#include <d3d12.h>

#define MAX_LIGHTS 16

namespace land_and_waves 
{
	struct VertexBaseData
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	struct VertexLightingData
	{
		XMFLOAT3 normal;
	};

	struct TreeSpriteVertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 size;
	};

	struct ParticleVertex
	{
		XMFLOAT3 startPos;
		XMFLOAT2 size;
		XMFLOAT3 selfLightColor;
		XMFLOAT3 startVelocity;
		float timeDelay;
	};

	struct PassConstants
	{
		XMFLOAT4X4 view = MathHelper::Identity4x4();
		XMFLOAT4X4 invView = MathHelper::Identity4x4();
		XMFLOAT4X4 projection = MathHelper::Identity4x4();
		XMFLOAT4X4 invProjection = MathHelper::Identity4x4();
		XMFLOAT4X4 viewProjection = MathHelper::Identity4x4();
		XMFLOAT4X4 invViewProjection = MathHelper::Identity4x4();
		XMFLOAT3 eyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		XMFLOAT2 renderTargetSize = { 0.0f, 0.0f };
		XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };
		float nearZ = 0.0f;
		float farZ = 0.0f;
		float totalTime = 0.0f;
		float deltaTime = 0.0f;
		XMFLOAT4 ambientLight;

		XMFLOAT4 gFogColor;
		float gFogStart;
		float gFogRange;
		XMFLOAT2 temp; // float4를 맞추기 위한 변수

		Light lights[MAX_LIGHTS];
	};

	struct ObjectConstants
	{
		XMFLOAT4X4 world = MathHelper::Identity4x4();
		XMFLOAT4X4 texTransform = MathHelper::Identity4x4();
	};

	struct MaterialConstants
	{
		XMFLOAT4 diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT3 fresnelR0 = { 0.01f, 0.01f, 0.01f };
		float roughness = 0.25f;

		XMFLOAT4X4 matTransform = MathHelper::Identity4x4();
	};

	// 파티클 상수
	struct ParticleConstants
	{
		float startTime;
		float lifeTime;
		float dragCoefficient;
	};


	struct LandAndWavesFrameResource
	{
		LandAndWavesFrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, UINT particleCount, UINT wavesVertexCount);
		LandAndWavesFrameResource(const LandAndWavesFrameResource& rhs) = delete;
		LandAndWavesFrameResource& operator=(const LandAndWavesFrameResource& rhs) = delete;
		~LandAndWavesFrameResource();

		// 커맨드 할당자는 GPU가 커맨드들을 다 처리 후 재설정해야 함
		// 프레임마다 할당자가 필요
		ComPtr<ID3D12CommandAllocator> cmdListAllocator;

		// 상수 버퍼는 그것을 참조하는 커맨드들을 GPU가 다 처리한 후 갱신해야 함
		// 프레임마다 상수 버퍼를 새로 만들어야 함
		std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB = nullptr;
		std::unique_ptr<UploadBuffer<MaterialConstants>> materialCB = nullptr;
		std::unique_ptr<UploadBuffer<ParticleConstants>> particleCB = nullptr;

		// 파도를 위한 버텍스 버퍼
		std::unique_ptr<UploadBuffer<VertexBaseData>> wavesBaseVB = nullptr;
		std::unique_ptr<UploadBuffer<VertexLightingData>> wavesLightingVB = nullptr;

		// GPU가 아직 이 프레임 자원들을 사용하고 있는지 판정하는 값
		UINT64 fence = 0;
	};
}

