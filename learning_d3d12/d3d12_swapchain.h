#pragma once
#include "stdafx.h"

namespace d3d12_init {

	void GetCleintSize(HWND hMainWnd, int& outWidth, int& outHeight)
	{
		RECT rcClient;
		::GetClientRect(hMainWnd, &rcClient);
		outWidth = rcClient.right - rcClient.left;
		outHeight = rcClient.bottom - rcClient.top;
	}

	DXGI_SWAP_CHAIN_DESC1 SetAndGetDxgiSwapChainDesc(int clientWidth, int clientHeight,
		bool isMsaa4xEnable, int msaa4xQualityLevels, int numSwapchainBuffers)
	{
		DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
		::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
		dxgiSwapChainDesc.Width = clientWidth;
		dxgiSwapChainDesc.Height = clientHeight;
		dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgiSwapChainDesc.SampleDesc.Count = (isMsaa4xEnable) ? 4 : 1;
		dxgiSwapChainDesc.SampleDesc.Quality = (isMsaa4xEnable) ? (msaa4xQualityLevels - 1) : 0;
		dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiSwapChainDesc.BufferCount = numSwapchainBuffers;
		dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
		dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		dxgiSwapChainDesc.Flags = 0;

		return dxgiSwapChainDesc;
	}
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC SetAndGetDxgiSwapChainFullScrenDesc()
	{
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScrenDesc;
		::ZeroMemory(&dxgiSwapChainFullScrenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
		dxgiSwapChainFullScrenDesc.RefreshRate.Numerator = 144;
		dxgiSwapChainFullScrenDesc.RefreshRate.Denominator = 1;
		dxgiSwapChainFullScrenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		dxgiSwapChainFullScrenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		dxgiSwapChainFullScrenDesc.Windowed = TRUE;

		return dxgiSwapChainFullScrenDesc;
	}

	IDXGISwapChain4* CreateSwapchain(HWND hMainWnd, IDXGIFactory6* dxgiFactory,
		ID3D12CommandQueue* d3dCommandQueue, DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc, 
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScrenDesc, UINT& outSwapChainBufferIndex)
	{
		IDXGISwapChain4* dxgiSwapchain = NULL;

		// 스왑 체인 생성
		HRESULT result = dxgiFactory->CreateSwapChainForHwnd(d3dCommandQueue, hMainWnd,
			&dxgiSwapChainDesc, &dxgiSwapChainFullScrenDesc, NULL, (IDXGISwapChain1**)&dxgiSwapchain);

		// ALT+Enter 비활성화
		dxgiFactory->MakeWindowAssociation(hMainWnd, DXGI_MWA_NO_ALT_ENTER);

		// 스왑체인의 현재 후면 버퍼 인덱스 저장
		outSwapChainBufferIndex = dxgiSwapchain->GetCurrentBackBufferIndex();

		return dxgiSwapchain;
	}
}