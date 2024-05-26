#pragma once
#include "stdafx.h"

namespace d3d12_init {

	std::pair<ID3D12DescriptorHeap*, ID3D12DescriptorHeap*> CreateDescriptorHeap(UINT numSwapchainBuffers, ID3D12Device* device,
		UINT& outRtvDescriptorIncrementSize, UINT& outDsvDescriptorIncrementSize)
	{
		ID3D12DescriptorHeap* rtvDescriptorHeap = NULL;

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
		::ZeroMemory(&descriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
		descriptorHeapDesc.NumDescriptors = numSwapchainBuffers;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;
		// 렌더 타겟 디스크립터 힙 생성
		HRESULT result = device->CreateDescriptorHeap(
			&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&rtvDescriptorHeap);
		// 렌더 타겟 디스크립터 힙의 원소 크기 저장
		outRtvDescriptorIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		ID3D12DescriptorHeap* dsvDescriptorHeap = NULL;

		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		// 깊이-스텐실 타겟 디스크립터 힙 생성
		result = device->CreateDescriptorHeap(
			&descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&dsvDescriptorHeap);
		// 깊이-스텐실 디스크립터 힙의 원소 크기 저장
		outDsvDescriptorIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		return std::pair<ID3D12DescriptorHeap*, ID3D12DescriptorHeap*>(rtvDescriptorHeap, dsvDescriptorHeap);
	}
}