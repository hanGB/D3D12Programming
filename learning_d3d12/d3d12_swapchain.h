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

	DXGI_SWAP_CHAIN_DESC SetAndGetSwapchainDesc(HWND hWnd, int clientWidth, int clientHeight,
		bool isMsaa4xEnable, int msaa4xQualityLevels, int numSwapchainBuffers)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		::ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		swapChainDesc.BufferCount = numSwapchainBuffers;
		swapChainDesc.BufferDesc.Width = clientWidth;
		swapChainDesc.BufferDesc.Height = clientHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 144;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = (isMsaa4xEnable) ? 4 : 1;
		swapChainDesc.SampleDesc.Quality = (isMsaa4xEnable) ? (msaa4xQualityLevels - 1) : 0;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		return swapChainDesc;
	}
	
	IDXGISwapChain4* CreateSwapchain(HWND hWnd, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue,
		DXGI_SWAP_CHAIN_DESC swapChainDesc, UINT& outSwapChainBufferIndex)
	{
		IDXGISwapChain4* swapChain = NULL;

		// 스왑 체인 생성
		HRESULT result = factory->CreateSwapChain(commandQueue, &swapChainDesc, (IDXGISwapChain**)&swapChain);
		// 스왑체인의 현재 후면 버퍼 인덱스 저장
		outSwapChainBufferIndex = swapChain->GetCurrentBackBufferIndex();

		// ALT+Enter 비활성화
		factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		return swapChain;
	}

	DXGI_MODE_DESC SetAndGetModeDesc(int clientWidth, int clientHeight)
	{
		DXGI_MODE_DESC modeDesc;
		::ZeroMemory(&modeDesc, sizeof(DXGI_MODE_DESC));
		modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		modeDesc.Width = clientWidth;
		modeDesc.Height = clientHeight;
		modeDesc.RefreshRate.Numerator = 144;
		modeDesc.RefreshRate.Denominator = 1;
		modeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		modeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		return modeDesc;
	}
}