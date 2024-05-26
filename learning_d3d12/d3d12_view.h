#pragma once
#include "stdafx.h"

namespace d3d12_init {

	ID3D12Resource* CreateRenerTargetView(IDXGISwapChain4* dxgiSwapchain, ID3D12Device* d3dDevice,
		ID3D12DescriptorHeap* d3dRtvDescriptorHeap, UINT m_rtvDescriptorIncrementSize, UINT index)
	{
		ID3D12Resource* d3dRenderTargetBuffer = NULL;

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvCPUDescriptorHandle.ptr += m_rtvDescriptorIncrementSize * index;

		HRESULT result = dxgiSwapchain->GetBuffer(index, __uuidof(ID3D12Resource), (void**)&d3dRenderTargetBuffer);
		d3dDevice->CreateRenderTargetView(d3dRenderTargetBuffer, NULL, d3dRtvCPUDescriptorHandle);

		return d3dRenderTargetBuffer;
	}

	ID3D12Resource* CreateDepthStencilView(ID3D12Device* d3dDevice,
		ID3D12DescriptorHeap* d3dDsvDescriptorHeap, int clientWidth, int clientHeight, 
		bool isMsaa4xEnable, UINT msaa4xQualityLevels)
	{
		ID3D12Resource* d3dDepthStencilTargetBuffer = NULL;

		D3D12_RESOURCE_DESC d3dResourceDesc;
		d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		d3dResourceDesc.Alignment = 0;
		d3dResourceDesc.Width = clientWidth;
		d3dResourceDesc.Height = clientHeight;
		d3dResourceDesc.DepthOrArraySize = 1;
		d3dResourceDesc.MipLevels = 1;
		d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		d3dResourceDesc.SampleDesc.Count = (isMsaa4xEnable) ? 4 : 1;
		d3dResourceDesc.SampleDesc.Quality = (isMsaa4xEnable) ? (msaa4xQualityLevels - 1) : 0;
		d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES d3dHeapProperties;
		::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
		d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		d3dHeapProperties.CreationNodeMask = 1;
		d3dHeapProperties.VisibleNodeMask = 1;

		D3D12_CLEAR_VALUE d3dClearValue;
		d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		d3dClearValue.DepthStencil.Depth = 1.0f;
		d3dClearValue.DepthStencil.Stencil = 0;
		
		// 깊이-스텐실 버퍼 생성
		HRESULT result = d3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&d3dDepthStencilTargetBuffer);

		// 깊이-스텐실 뷰 생성
		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = d3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dDevice->CreateDepthStencilView(d3dDepthStencilTargetBuffer, NULL, d3dDsvCPUDescriptorHandle);

		return d3dDepthStencilTargetBuffer;
	}
}