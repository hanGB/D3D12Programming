#pragma once
#include "stdafx.h"

namespace d3d12_init {

	IDXGIFactory6* CreateFactory()
	{
		HRESULT result;

		UINT factoryFlags = 0;
		IDXGIFactory6* factory = NULL;

#ifdef PER_DEBUG
		ID3D12Debug* debugController = NULL;
		result = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&debugController);
		if (debugController)
		{
			debugController->EnableDebugLayer();
			debugController->Release();
		}
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif 

		result = ::CreateDXGIFactory2(factoryFlags, __uuidof(IDXGIFactory6), (void**)&factory);

		return factory;
	}

	ID3D12Device* CreateDevice(IDXGIFactory6* factory)
	{
		ID3D12Device* device = NULL;

		IDXGIAdapter1* adapter = NULL;
		// 모든 하드웨어 어댑터에 대하여 특성 레벨 12.0를 지원하는 하드웨어 디바이스를 생성
		for (UINT i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i)
		{
			DXGI_ADAPTER_DESC1 adapterDesc;
			adapter->GetDesc1(&adapterDesc);
			if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2, __uuidof(ID3D12Device), (void**)&device))) break;
		}

		// 특성 레벨 12.2를 지우너하는 하드웨어 디바이스를 생성할 수 없으면 WARP 디바이스 생성
		if (!adapter)
		{
			factory->EnumWarpAdapter(__uuidof(IDXGIFactory4), (void**)&adapter);
			D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&device);
		}

		if (adapter) adapter->Release();

		return device;
	}

	void SetViewportAndScissorRect(D3D12_VIEWPORT& outViewport, D3D12_RECT& outScissorRect, int clientWidth, int clientHeight)
	{
		// 뷰포트를 주 윈도우의 클라이언트 영역 전체로 설정
		outViewport.TopLeftX = 0;
		outViewport.TopLeftY = 0;
		outViewport.Width = static_cast<float>(clientWidth);
		outViewport.Height = static_cast<float>(clientHeight);
		outViewport.MinDepth = 0.f;
		outViewport.MaxDepth = 1.f;

		// 씨저 사각형을 주 윈도우의 클라이언트 영역 전체로 설정
		outScissorRect = { 0, 0, clientWidth, clientHeight };
	}

	void SetMsaa(ID3D12Device* device, bool& outMsaa4xEnable, UINT& outMsaa4xQualityLevels)
	{
		// 디바이스 가 지원하는 다중 샘플의 품질 수준 확인 후 1보다 크면 활성화
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQulityLevels;
		msaaQulityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQulityLevels.SampleCount = 4; // MSAA 4x
		msaaQulityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQulityLevels.NumQualityLevels = 0;
		device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msaaQulityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
		outMsaa4xQualityLevels = msaaQulityLevels.NumQualityLevels;
		outMsaa4xEnable = (outMsaa4xQualityLevels > 1) ? true : false;
	}

	ID3D12Fence* CreateFence(ID3D12Device* device, UINT64& outFenceValue)
	{
		ID3D12Fence* d3dFence = NULL;

		// 펜스 생성 및 펜스 값을 0으로 설정
		HRESULT result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&d3dFence);
		outFenceValue = 0;

		return d3dFence;
	}

	HANDLE CreateFenceEvent()
	{
		// 펜스와 동기화를 위한 이벤트 객체 생성: 초기값은 FALSE, 이벤트 실행시 자동으로 FALSE가 됨
		return ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}
}
