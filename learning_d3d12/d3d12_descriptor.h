#pragma once
#include "stdafx.h"

namespace d3d12_init {

	std::pair<ID3D12DescriptorHeap*, ID3D12DescriptorHeap*> CreateDescriptorHeap(UINT numSwapchainBuffers, ID3D12Device* d3dDevice,
		UINT& outRtvDescriptorIncrementSize, UINT& outDsvDescriptorIncrementSize)
	{
		ID3D12DescriptorHeap* d3dRtvDescriptorHeap = NULL;

		D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
		::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
		d3dDescriptorHeapDesc.NumDescriptors = numSwapchainBuffers;
		d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		d3dDescriptorHeapDesc.NodeMask = 0;
		// 렌더 타겟 디스크립터 힙 생성
		HRESULT hResult = d3dDevice->CreateDescriptorHeap(
			&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&d3dRtvDescriptorHeap);
		// 렌더 타겟 디스크립터 힙의 원소 크기 저장
		outRtvDescriptorIncrementSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		ID3D12DescriptorHeap* d3dDsvDescriptorHeap = NULL;

		d3dDescriptorHeapDesc.NumDescriptors = 1;
		d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		// 깊이-스텐실 타겟 디스크립터 힙 생성
		hResult = d3dDevice->CreateDescriptorHeap(
			&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&d3dDsvDescriptorHeap);
		// 깊이-스텐실 디스크립터 힙의 원소 크기 저장
		outDsvDescriptorIncrementSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		return std::pair<ID3D12DescriptorHeap*, ID3D12DescriptorHeap*>(d3dRtvDescriptorHeap, d3dDsvDescriptorHeap);
	}
}