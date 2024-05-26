#pragma once
#include "stdafx.h"

namespace d3d12_init {

	IDXGIFactory6* CreateFactory()
	{
		HRESULT hResult;

		UINT dxgiFactoryFlags = 0;
		IDXGIFactory6* dxgiFactory = NULL;

#ifdef PER_DEBUG
		ID3D12Debug* d3dDebugController = NULL;
		hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&d3dDebugController);
		if (d3dDebugController)
		{
			d3dDebugController->EnableDebugLayer();
			d3dDebugController->Release();
		}
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif 

		hResult = ::CreateDXGIFactory2(dxgiFactoryFlags, __uuidof(IDXGIFactory6), (void**)&dxgiFactory);

		return dxgiFactory;
	}

	ID3D12Device* CreateDevice(IDXGIFactory6* dxgiFactory)
	{
		ID3D12Device* d3dDevice = NULL;

		IDXGIAdapter1* d3dAdapter = NULL;
		// 모든 하드웨어 어댑터에 대하여 특성 레벨 12.0를 지원하는 하드웨어 디바이스를 생성
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(i, &d3dAdapter); ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
			d3dAdapter->GetDesc1(&dxgiAdapterDesc);
			if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(d3dAdapter, D3D_FEATURE_LEVEL_12_2, __uuidof(ID3D12Device), (void**)&d3dDevice))) break;
		}

		// 특성 레벨 12.2를 지우너하는 하드웨어 디바이스를 생성할 수 없으면 WARP 디바이스 생성
		if (!d3dAdapter)
		{
			dxgiFactory->EnumWarpAdapter(__uuidof(IDXGIFactory4), (void**)&d3dAdapter);
			D3D12CreateDevice(d3dAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&d3dDevice);
		}

		if (d3dAdapter) d3dAdapter->Release();

		return d3dDevice;
	}

	void SetViewportAndScissorRect(D3D12_VIEWPORT& outD3dViewport, D3D12_RECT& outD3dScissorRect, int clientWidth, int clientHeight)
	{
		// 뷰포트를 주 윈도우의 클라이언트 영역 전체로 설정
		outD3dViewport.TopLeftX = 0;
		outD3dViewport.TopLeftY = 0;
		outD3dViewport.Width = static_cast<float>(clientWidth);
		outD3dViewport.Height = static_cast<float>(clientHeight);
		outD3dViewport.MinDepth = 0.f;
		outD3dViewport.MaxDepth = 1.f;

		// 씨저 사각형을 주 윈도우의 클라이언트 영역 전체로 설정
		outD3dScissorRect = { 0, 0, clientWidth, clientHeight };
	}

	void SetMsaa(ID3D12Device* d3dDevice, bool& outMsaa4xEnable, UINT& outMsaa4xQualityLevels)
	{
		// 디바이스 가 지원하는 다중 샘플의 품질 수준 확인 후 1보다 크면 활성화
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQulityLevels;
		d3dMsaaQulityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dMsaaQulityLevels.SampleCount = 4; // MSAA 4x
		d3dMsaaQulityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		d3dMsaaQulityLevels.NumQualityLevels = 0;
		d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&d3dMsaaQulityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
		outMsaa4xQualityLevels = d3dMsaaQulityLevels.NumQualityLevels;
		outMsaa4xEnable = (outMsaa4xQualityLevels > 1) ? true : false;
	}

	ID3D12Fence* CreateFence(ID3D12Device* d3dDevice, UINT64& outFenceValue)
	{
		ID3D12Fence* d3dFence = NULL;

		// 펜스 생성 및 펜스 값을 0으로 설정
		HRESULT hResult = d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&d3dFence);
		outFenceValue = 0;

		return d3dFence;
	}

	HANDLE CreateFenceEvent()
	{
		// 펜스와 동기화를 위한 이벤트 객체 생성: 초기값은 FALSE, 이벤트 실행시 자동으로 FALSE가 됨
		return ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}
}
