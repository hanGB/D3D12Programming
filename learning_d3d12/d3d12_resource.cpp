#include "stdafx.h"
#include "d3d12_resource.h"

D3D12_HEAP_PROPERTIES d3d12_init::SetAndGetHeapPropertiesForBufferResource(D3D12_HEAP_TYPE heapType)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	::ZeroMemory(&heapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	heapProperties.Type = heapType;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	return heapProperties;
}

D3D12_RESOURCE_DESC d3d12_init::SetAndGetResourceDescForBufferResource(UINT numBytes)
{
	D3D12_RESOURCE_DESC resourceDesc;
	::ZeroMemory(&resourceDesc, sizeof(D3D12_RESOURCE_DESC));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = numBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	return resourceDesc;
}

D3D12_RESOURCE_BARRIER d3d12_init::SetAndGetResourceBarrierForBufferResource(ID3D12Resource* buffer, D3D12_RESOURCE_STATES resourceStates)
{
	D3D12_RESOURCE_BARRIER resourceBarrier;
	::ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = buffer;
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	resourceBarrier.Transition.StateAfter = resourceStates;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	return resourceBarrier;
}

void d3d12_init::CopyDataToBuffer(ID3D12Resource** buffer, void* data, UINT numBytes)
{
	D3D12_RANGE readRange = { 0, 0 };
	UINT8* bufferDataBegin = NULL;
	(*buffer)->Map(0, &readRange, (void**)&bufferDataBegin);
	memcpy(bufferDataBegin, data, numBytes);
	(*buffer)->Unmap(0, NULL);
}

void d3d12_init::CreateUploadBufferAndCopyToDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, 
	ID3D12Resource** uploadBuffer, ID3D12Resource** buffer, D3D12_HEAP_PROPERTIES heapProperties, 
	D3D12_RESOURCE_DESC resourceDesc, D3D12_RESOURCE_STATES resourceStates, void* data, UINT numBytes)
{
	// 업로드 버퍼 생성
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	HRESULT result = device->CreateCommittedResource(
		&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL, __uuidof(ID3D12Resource), (void**)uploadBuffer);

	// 업로드 버퍼에 데이터 복사
	CopyDataToBuffer(uploadBuffer, data, numBytes);

	// 업로드 버퍼의 내용을 디폴트 버퍼에 복사
	commandList->CopyResource(*buffer, *uploadBuffer);
	D3D12_RESOURCE_BARRIER resourceBarrier = SetAndGetResourceBarrierForBufferResource((*buffer), resourceStates);
	commandList->ResourceBarrier(1, &resourceBarrier);
}

ID3D12Resource* d3d12_init::CreateBufferResource(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, 
	void* data, UINT numBytes, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceStates, ID3D12Resource** uploadeBuffer)
{
	ID3D12Resource* buffer = NULL;

	D3D12_HEAP_PROPERTIES heapProperties = SetAndGetHeapPropertiesForBufferResource(heapType);
	D3D12_RESOURCE_DESC resourceDesc = SetAndGetResourceDescForBufferResource(numBytes);

	D3D12_RESOURCE_STATES resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;
	if (heapType == D3D12_HEAP_TYPE_UPLOAD) resourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ;
	else if (heapType == D3D12_HEAP_TYPE_READBACK) resourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST;

	HRESULT result = device->CreateCommittedResource(
		&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceInitialStates,
		NULL, __uuidof(ID3D12Resource), (void**)&buffer);

	if (data)
	{
		switch (heapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT:
		{
			if (uploadeBuffer)
			{
				CreateUploadBufferAndCopyToDefaultBuffer(device, commandList, uploadeBuffer, &buffer,
					heapProperties, resourceDesc, resourceStates, data, numBytes);
			}
			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:
		{
			CopyDataToBuffer(&buffer, data, numBytes);
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:
			break;
		}
	}

	return buffer;
}
