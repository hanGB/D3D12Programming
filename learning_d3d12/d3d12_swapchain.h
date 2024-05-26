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

	DXGI_SWAP_CHAIN_DESC SetAndGetDxgiSwapChainDesc(HWND hWnd, int clientWidth, int clientHeight,
		bool isMsaa4xEnable, int msaa4xQualityLevels, int numSwapchainBuffers)
	{
		DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
		::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		dxgiSwapChainDesc.BufferCount = numSwapchainBuffers;
		dxgiSwapChainDesc.BufferDesc.Width = clientWidth;
		dxgiSwapChainDesc.BufferDesc.Height = clientHeight;
		dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 144;
		dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		dxgiSwapChainDesc.OutputWindow = hWnd;
		dxgiSwapChainDesc.SampleDesc.Count = (isMsaa4xEnable) ? 4 : 1;
		dxgiSwapChainDesc.SampleDesc.Quality = (isMsaa4xEnable) ? (msaa4xQualityLevels - 1) : 0;
		dxgiSwapChainDesc.Windowed = TRUE;
		dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		return dxgiSwapChainDesc;
	}
	
	IDXGISwapChain4* CreateSwapchain(HWND hWnd, IDXGIFactory6* dxgiFactory, ID3D12CommandQueue* d3dCommandQueue,
		DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc, UINT& outSwapChainBufferIndex)
	{
		IDXGISwapChain4* dxgiSwapchain = NULL;

		// 스왑 체인 생성
		HRESULT result = dxgiFactory->CreateSwapChain(d3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&dxgiSwapchain);
		// 스왑체인의 현재 후면 버퍼 인덱스 저장
		outSwapChainBufferIndex = dxgiSwapchain->GetCurrentBackBufferIndex();

		// ALT+Enter 비활성화
		dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		return dxgiSwapchain;
	}

	DXGI_MODE_DESC SetAndGetDxgiModeDesc(int clientWidth, int clientHeight)
	{
		DXGI_MODE_DESC dxgiModeDesc;
		::ZeroMemory(&dxgiModeDesc, sizeof(DXGI_MODE_DESC));
		dxgiModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgiModeDesc.Width = clientWidth;
		dxgiModeDesc.Height = clientHeight;
		dxgiModeDesc.RefreshRate.Numerator = 144;
		dxgiModeDesc.RefreshRate.Denominator = 1;
		dxgiModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		dxgiModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		return dxgiModeDesc;
	}
}