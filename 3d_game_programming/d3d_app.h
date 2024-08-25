#pragma once

#include "d3d_util.h"
#include "game_timer.h"

class D3DApp
{
protected:
	// 생성자, 소멸자
	D3DApp(HINSTANCE hInstance);
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp& rhs) = delete;
	virtual ~D3DApp();

public:
	static D3DApp* GetApp();

	HINSTANCE	AppInst() const;
	HWND		MainWnd() const;
	float		AspectRatio() const;

	// MSAA 설정
	bool Get4xMsaaState() const;
	void Set4xMsaaState(bool value);

	int Run();

	virtual bool Initialize();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	// 렌더, 깊이 스텐실 
	virtual void CreateRtvAndDsvDescriptorHeaps();
	virtual void OnResize();
	virtual void Update(const GameTimer& gt) = 0;
	virtual void Draw(const GameTimer& gt) = 0;

	// 마우스 사용
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }
	// 키보드 사용
	virtual void OnKeyboradUse(WPARAM btnState, bool isPressed) { }
	
	// 윈도우, 다이렉트3D 초기화
	bool InitMainWindow();
	bool InitDrect3D();
	void CreateCommandObjects();
	void CreateSwapChain();

	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	// 프레임
	void CalculateFrameStats();

	// 로그
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

	static const int c_SWAP_CHAIN_BUFFER_COUNT = 2;

	static D3DApp* m_app;

	HINSTANCE m_hAppInst = nullptr;
	HWND m_hMainWnd = nullptr;
	
	// app 상태
	bool m_appPause = false;
	bool m_minimized = false;
	bool m_maximized = false;
	bool m_resizing = false;
	bool m_fullscreenState = false;

	// msaa
	bool m_4xMsaaState = false;
	UINT m_4xMsaaQuality = 0;

	GameTimer m_timer;
	
	// 다이렉트3D
	ComPtr<IDXGIFactory4> m_dxgiFactory;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D12Device> m_d3dDevice;

	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_currentFence = 0;

	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandListAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	int m_currentBackBuffer = 0;
	ComPtr<ID3D12Resource> m_swapChainBuffer[c_SWAP_CHAIN_BUFFER_COUNT];
	ComPtr<ID3D12Resource> m_depthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	UINT m_cbvSrvDescriptorSize = 0;

	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_scissorRect;
	
	// 설정값
	std::wstring m_mainWndCaption = L"d3d App";
	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int m_clientWidth = 1920;//.0 / 1.5;
	int m_clientHeight = 1080;//.0 / 1.5;
};