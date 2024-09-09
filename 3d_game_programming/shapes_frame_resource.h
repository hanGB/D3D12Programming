#pragma once
#include "d3d_util.h"
#include "upload_buffer.h"
#include "math_helper.h"

namespace shapes
{
	struct VertexPosData
	{
		XMFLOAT3 pos;
	};

	struct VertexColorData
	{
		XMFLOAT3 normal;
		XMFLOAT4 color;
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
	};

	struct ObjectConstants
	{
		XMFLOAT4X4 world = MathHelper::Identity4x4();
	};

	struct ShapesFrameResource
	{
		ShapesFrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
		ShapesFrameResource(const ShapesFrameResource& rhs) = delete;
		ShapesFrameResource& operator=(const ShapesFrameResource& rhs) = delete;
		~ShapesFrameResource();

		// 커맨드 할당자는 GPU가 커맨드들을 다 처리 후 재설정해야 함
		// 프레임마다 할당자가 필요
		ComPtr<ID3D12CommandAllocator> cmdListAllocator;

		// 상수 버퍼는 그것을 참조하는 커맨드들을 GPU가 다 처리한 후 갱신해야 함
		// 프레임마다 상수 버퍼를 새로 만들어야 함
		std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB = nullptr;

		// GPU가 아직 이 프레임 자원들을 사용하고 있는지 판정하는 값
		UINT64 fence = 0;
	};

}
