#pragma once
#include "stdafx.h"

namespace d3d12_init {

	ID3D12Resource* CreateRenerTargetView(IDXGISwapChain4* swapChain, ID3D12Device* device,
		ID3D12DescriptorHeap* rtvDescriptorHeap, UINT m_rtvDescriptorIncrementSize, UINT index)
	{
		ID3D12Resource* renderTargetBuffer = NULL;

		D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvCPUDescriptorHandle.ptr += m_rtvDescriptorIncrementSize * index;

		HRESULT result = swapChain->GetBuffer(index, __uuidof(ID3D12Resource), (void**)&renderTargetBuffer);
		device->CreateRenderTargetView(renderTargetBuffer, NULL, rtvCPUDescriptorHandle);

		return renderTargetBuffer;
	}

	ID3D12Resource* CreateDepthStencilView(ID3D12Device* device,
		ID3D12DescriptorHeap* dsvDescriptorHeap, int clientWidth, int clientHeight, 
		bool isMsaa4xEnable, UINT msaa4xQualityLevels)
	{
		ID3D12Resource* depthStencilTargetBuffer = NULL;

		D3D12_RESOURCE_DESC resourceDesc;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = clientWidth;
		resourceDesc.Height = clientHeight;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		resourceDesc.SampleDesc.Count = (isMsaa4xEnable) ? 4 : 1;
		resourceDesc.SampleDesc.Quality = (isMsaa4xEnable) ? (msaa4xQualityLevels - 1) : 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES heapProperties;
		::ZeroMemory(&heapProperties, sizeof(D3D12_HEAP_PROPERTIES));
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;
		
		// 깊이-스텐실 버퍼 생성
		HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, __uuidof(ID3D12Resource), (void**)&depthStencilTargetBuffer);

		// 깊이-스텐실 뷰 생성
		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		device->CreateDepthStencilView(depthStencilTargetBuffer, NULL, d3dDsvCPUDescriptorHandle);

		return depthStencilTargetBuffer;
	}
}