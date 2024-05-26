#include "stdafx.h"
#include "d3d12_renderer.h"
#include "d3d12_swapchain.h"
#include "d3d12_device.h"
#include "d3d12_command.h"
#include "d3d12_descriptor.h"
#include "d3d12_view.h"

D3D12Renderer::D3D12Renderer()
{
	m_clientWidth = PER_DEFAULT_WINDOW_WIDTH;
	m_clientHeight = PER_DEFAULT_WINDOW_HEIGHT;

	m_dxgiFactory = NULL;
	m_dxgiSwapchain = NULL;
	m_d3dDevice = NULL;

	m_d3dCommandQueue = NULL;
	m_d3dCommandAllocator = NULL;
	m_d3dCommandList = NULL;
	m_d3dPipelineState = NULL;

	for (int i = 0; i < c_NUM_SWAP_CAHIN_BUFFERS; ++i) m_d3dRenderTargetBuffers[i] = NULL;
	m_d3dRtvDescriptorHeap = NULL;
	m_rtvDescriptorIncrementSize = 0;

	m_d3dDepthStencilTargetBuffer = NULL;
	m_d3dDsvDescriptorHeap = NULL;
	m_dsvDescriptorIncrementSize = 0;

	m_swapChainBufferIndex = 0;

	m_d3dFence = NULL;
	m_fenceValue = 0;
	m_fenceEvent = NULL;
}

D3D12Renderer::~D3D12Renderer()
{

}

void D3D12Renderer::CreateInterface(HWND hMainWnd)
{
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateSwapchain(hMainWnd);
	CreateRtvAndDsvDescriptorHeaps();
	
	CreateRenderTargetViews();
	CreateDepthStencilView();

	PERLog::Logger().Info("Direct3D 12 인터페이스 생성 완료");
}

void D3D12Renderer::ReleaseInterface()
{
	for (int i = 0; i < c_NUM_SWAP_CAHIN_BUFFERS; ++i) 
		if (m_d3dRenderTargetBuffers[i]) m_d3dRenderTargetBuffers[i]->Release();
	
	if (m_d3dRtvDescriptorHeap) m_d3dRtvDescriptorHeap->Release();
	if (m_d3dDepthStencilTargetBuffer) m_d3dDepthStencilTargetBuffer->Release();
	if (m_d3dDsvDescriptorHeap) m_d3dDsvDescriptorHeap->Release();

	if (m_d3dCommandAllocator) m_d3dCommandAllocator->Release();
	if (m_d3dCommandQueue) m_d3dCommandQueue->Release();
	if (m_d3dPipelineState) m_d3dPipelineState->Release();
	if (m_d3dCommandList )m_d3dCommandList->Release();

	if (m_d3dFence) m_d3dFence->Release();

	if (m_dxgiSwapchain) m_dxgiSwapchain->Release();
	if (m_dxgiFactory) m_dxgiFactory->Release();
	if (m_d3dDevice) m_d3dDevice->Release();

#ifdef PER_DEBUG
	IDXGIDebug1* dxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&dxgiDebug);
	HRESULT hResult = dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	dxgiDebug->Release();
#endif 
	PERLog::Logger().Info("Direct3D 12 인테페이스 삭제 완료");
}

void D3D12Renderer::FrameAdvance()
{
	// 커맨드 할당자, 커맨드 리스트 리셋
	HRESULT hResult = m_d3dCommandAllocator->Reset();
	hResult = m_d3dCommandList->Reset(m_d3dCommandAllocator, NULL);

	// 현재 렌더 타켓에 대한 프리젠트가 끝나길 대기
	// 프리젠트가 끝나면 렌더 타겟 버퍼의 상태는 프리젠트 상태에서 렌더 타겟 상태로 바뀜
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_d3dRenderTargetBuffers[m_swapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_d3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	// 뷰포트와 씨저 사각형 설정
	m_d3dCommandList->RSSetViewports(1, &m_d3dViewport);
	m_d3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);

	// 현재의 렌더 타겟에 해당하는 서술자의 CPU 주소(핸들) 계산
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += m_swapChainBufferIndex * m_rtvDescriptorIncrementSize;
	// 원하는 색상으로 렌더 타켓(뷰) 클리어
	float clearColor[4] = { 0.f, 0.125f, 0.3f, 1.f }; // Colors::Azure
	m_d3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, clearColor, 0, NULL);

	// 깊이-스텐실 디스크립터의 CPU 주소 계산
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_d3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// 원하는 값으로 깊이-스텐실(뷰) 클리어
	m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);

	// 렌더 타겟 뷰(디스크립터)와 깊이-스텐실 뷰(디스크립터)를 출력-병합 단계(OM)에 연결
	m_d3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	// 렌더 코드 추가 위치


	// 현재 렌더 타겟이 렌더링 끝나길 대기
	// GPU가 렌더 타켓(버퍼)를 더 이상 사용하지 않으면 렌더 타겟의 상태는 프리젠트 상태로 바뀜
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_d3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	// 커맨드 리스트를 닫힌 상태로 변경
	hResult = m_d3dCommandList->Close();

	// 커맨드 리스트를 커멘드 큐에 추가하여 실행
	ID3D12CommandList* d3dCommandLists[] = { m_d3dCommandList };
	m_d3dCommandQueue->ExecuteCommandLists(1, d3dCommandLists);

	// GPU가 모든 커맨드 리스트를 실행할 때 까지 대기
	WaitForGpuComplete();

	// 스왑체인 프리젠트
	// 프리젠트 후 현재 렌더 타겟(후면 버퍼)의 내용이 전면 버퍼로 옮겨지고 렌더 타겟 인덱스가 변경됨
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_dxgiSwapchain->Present1(1, 0, &dxgiPresentParameters);
	m_swapChainBufferIndex = m_dxgiSwapchain->GetCurrentBackBufferIndex();

	m_dxgiSwapchain->Present(0, 0);
}

void D3D12Renderer::SetClientSize(int width, int height)
{
	m_clientWidth = width;
	m_clientHeight = height;
}

void D3D12Renderer::WaitForGpuComplete()
{
	// CPU 펜스 값 증가
	m_fenceValue++;
	// GPU가 펜스의 값을 설정하는 커맨드를 커맨드 큐에 추가
	const UINT64 fence = m_fenceValue;
	HRESULT hResult = m_d3dCommandQueue->Signal(m_d3dFence, fence);

	if (m_d3dFence->GetCompletedValue() < fence)
	{
		// 펜스의 현재 값이 설정한 값보다 작으면 펜스의 현재 값이 될 때까지 대기
		hResult = m_d3dFence->SetEventOnCompletion(fence, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void D3D12Renderer::CreateDirect3DDevice()
{
	// 팩토리 생성
	m_dxgiFactory = d3d12_init::CreateFactory();
	// 디바이스 생성
	m_d3dDevice = d3d12_init::CreateDevice(m_dxgiFactory);

	// 뷰포트 써저 설정
	d3d12_init::SetViewportAndScissorRect(m_d3dViewport, m_d3dScissorRect, m_clientWidth, m_clientHeight);
	// MSAA 설정
	d3d12_init::SetMsaa(m_d3dDevice, m_isMsaa4xEnable, m_msaa4xQualityLevels);
	// 펜스 생성
	m_d3dFence = d3d12_init::CreateFence(m_d3dDevice, m_fenceValue);
	// 펜스 이벤트 생성
	m_fenceEvent = d3d12_init::CreateFenceEvent();
}

void D3D12Renderer::CreateCommandQueueAndList()
{
	// 커맨드 관련 인터페이스 생성
	m_d3dCommandQueue = d3d12_init::CreateCommandQueue(m_d3dDevice);
	m_d3dCommandAllocator = d3d12_init::CreateCommandAllocator(m_d3dDevice);
	m_d3dCommandList = d3d12_init::CreateCommandList(m_d3dDevice, m_d3dCommandAllocator);
}

void D3D12Renderer::CreateSwapchain(HWND hMainWnd)
{
	// 클라이언트 크기 받기
	d3d12_init::GetCleintSize(hMainWnd, m_clientWidth, m_clientHeight);

	// 스왑 체인 정보 설정 밎 받기
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc = d3d12_init::SetAndGetDxgiSwapChainDesc(m_clientWidth, m_clientHeight,
		m_isMsaa4xEnable, m_msaa4xQualityLevels, c_NUM_SWAP_CAHIN_BUFFERS);
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScrenDesc = d3d12_init::SetAndGetDxgiSwapChainFullScrenDesc();

	// 스왑 체인 생성
	m_dxgiSwapchain = d3d12_init::CreateSwapchain(hMainWnd, m_dxgiFactory, m_d3dCommandQueue,
		dxgiSwapChainDesc, dxgiSwapChainFullScrenDesc, m_swapChainBufferIndex);
}

void D3D12Renderer::CreateRtvAndDsvDescriptorHeaps()
{
	// 렌더 타겟, 깊이-스텐실 디스크립터 힙 생성
	std::pair<ID3D12DescriptorHeap*, ID3D12DescriptorHeap*> heaps = d3d12_init::CreateDescriptorHeap(
		c_NUM_SWAP_CAHIN_BUFFERS, m_d3dDevice, m_rtvDescriptorIncrementSize, m_dsvDescriptorIncrementSize);
	m_d3dRtvDescriptorHeap = heaps.first;
	m_d3dDsvDescriptorHeap = heaps.second;
}

void D3D12Renderer::CreateRenderTargetViews()
{
	// 렌더 타겟 버퍼, 뷰 생성
	for (UINT i = 0; i < c_NUM_SWAP_CAHIN_BUFFERS; ++i) {
		m_d3dRenderTargetBuffers[i] = d3d12_init::CreateRenerTargetView(m_dxgiSwapchain, m_d3dDevice,
			m_d3dRtvDescriptorHeap, m_rtvDescriptorIncrementSize, i);
	}
}

void D3D12Renderer::CreateDepthStencilView()
{
	// 깊이-스텐실 버퍼, 뷰 생성
	m_d3dDepthStencilTargetBuffer = d3d12_init::CreateDepthStencilView(m_d3dDevice, m_d3dDsvDescriptorHeap,
		m_clientWidth, m_clientHeight, m_isMsaa4xEnable, m_msaa4xQualityLevels);
}
