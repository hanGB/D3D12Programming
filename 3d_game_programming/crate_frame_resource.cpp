#include "stdafx.h"
#include "crate_frame_resource.h"

crate::CrateFrameResource::CrateFrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdListAllocator)));

	passCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	objectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
	materialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
}

crate::CrateFrameResource::~CrateFrameResource()
{

}