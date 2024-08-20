#pragma once
#include "d3d_util.h"
#include "upload_buffer.h"

struct PassConstants
{

};

struct ObjectConstants
{

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
	UINT64 fence;
};
