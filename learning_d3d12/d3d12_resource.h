#pragma once
#include "stdafx.h"

namespace d3d12_init {

	// 버퍼 리소스를 만들기 위한 힙 정보 설정
	D3D12_HEAP_PROPERTIES SetAndGetHeapPropertiesForBufferResource(D3D12_HEAP_TYPE heapType);
	// 버퍼 리소스를 만들기 위한 리소스 정보 설정
	D3D12_RESOURCE_DESC SetAndGetResourceDescForBufferResource(UINT numBytes);
	// 리소스 베리어 설정
	D3D12_RESOURCE_BARRIER SetAndGetResourceBarrierForBufferResource(ID3D12Resource* buffer, D3D12_RESOURCE_STATES resourceStates);

	// 버퍼에 데이터 복사
	void CopyDataToBuffer(ID3D12Resource** buffer, void* data, UINT numBytes);

	// 업로드 버퍼를 만들어 디폴트 버퍼에 복사
	void CreateUploadBufferAndCopyToDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
		ID3D12Resource** uploadBuffer, ID3D12Resource** buffer,
		D3D12_HEAP_PROPERTIES heapProperties, D3D12_RESOURCE_DESC resourceDesc,
		D3D12_RESOURCE_STATES resourceStates,
		void* data, UINT numBytes);

	// 버퍼 리소스 생성
	ID3D12Resource* CreateBufferResource(
		ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
		void* data, UINT numBytes, D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_STATES resourceStates, ID3D12Resource** uploadeBuffer);

}