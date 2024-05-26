#pragma once

class D3D12Renderer
{
public:
	D3D12Renderer();
	~D3D12Renderer();

	// 각종 D3D12 인터페이스를 생성, 삭제
	void CreateInterface(HWND hMainWnd);
	void ReleaseInterface();

	void FrameAdvance();

	void SetClientSize(int width, int height);
	void WaitForGpuComplete();

private:
	// 디바이스, 커맨트 큐와 리스트, 스왑 체인, 디스크립터 힙 생성
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
	void CreateSwapchain(HWND hMainWnd);
	void CreateRtvAndDsvDescriptorHeaps();

	// 렌터 타켓 뷰, 깊이-스텐실 뷰 생성
	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	// 스왑 체인 후면 버퍼 개수
	static const UINT	c_NUM_SWAP_CAHIN_BUFFERS = 2;

	// 클라이언트 크기
	int m_clientWidth;
	int m_clientHeight;

	// DXGI 팩토리
	IDXGIFactory6*		m_dxgiFactory;
	// Direct3D 디바이스(리소스 생성)
	ID3D12Device* m_d3dDevice;
	// 스왑 체인(디스플레이 제어)
	IDXGISwapChain4*	m_dxgiSwapchain;

	// MSAA 다중 샘플링 설정
	bool				m_isMsaa4xEnable = false;
	UINT				m_msaa4xQualityLevels = 0;

	// 현재 스왑 체인 후면 버퍼 인덱스
	UINT				m_swapChainBufferIndex;

	// 렌더 타켓 버퍼, 디스크립터 힙
	ID3D12Resource*				m_d3dRenderTargetBuffers[c_NUM_SWAP_CAHIN_BUFFERS];
	ID3D12DescriptorHeap*		m_d3dRtvDescriptorHeap;
	// 렌더 타켓 디스크립터 원소 크기
	UINT						m_rtvDescriptorIncrementSize;

	// 깊이-스텐실 타켓 버퍼, 디스크립터 힙
	ID3D12Resource*				m_d3dDepthStencilTargetBuffer;
	ID3D12DescriptorHeap*		m_d3dDsvDescriptorHeap;
	// 깊이-스텐실 디스크립터 원소 크기
	UINT						m_dsvDescriptorIncrementSize;

	// 커맨드 큐, 할당자, 리스트
	ID3D12CommandQueue*			m_d3dCommandQueue;
	ID3D12CommandAllocator*		m_d3dCommandAllocator;
	ID3D12GraphicsCommandList*	m_d3dCommandList;

	// 그래픽스 파이프라인 상태 객체
	ID3D12PipelineState* m_d3dPipelineState;

	// 펜스
	ID3D12Fence*	m_d3dFence;
	UINT64			m_fenceValue;
	HANDLE			m_fenceEvent;

	// 뷰포트, 씨저
	D3D12_VIEWPORT	m_d3dViewport;
	D3D12_RECT		m_d3dScissorRect;
};