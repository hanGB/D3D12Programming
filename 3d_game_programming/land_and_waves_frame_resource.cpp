#include "stdafx.h"
#include "land_and_waves_frame_resource.h"

land_and_waves::LandAndWavesFrameResource::LandAndWavesFrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, UINT particleCount, UINT wavesVertexCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdListAllocator)));

	passCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	objectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
	materialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
	particleCB = std::make_unique<UploadBuffer<ParticleConstants>>(device, particleCount, true);

	wavesBaseVB = std::make_unique<UploadBuffer<VertexBaseData>>(device, wavesVertexCount, false);
	wavesLightingVB = std::make_unique<UploadBuffer<VertexLightingData>>(device, wavesVertexCount, false);
}

land_and_waves::LandAndWavesFrameResource::~LandAndWavesFrameResource()
{

}
