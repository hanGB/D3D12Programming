#include "stdafx.h"
#include "d3d12_renderer.h"
#include "d3d12_swapChain.h"
#include "d3d12_device.h"
#include "d3d12_command.h"
#include "d3d12_descriptor.h"
#include "d3d12_view.h"
#include "d3d12_camera.h"
#include "per_world.h"
#include "per_player.h"
#include "graphics_component.h"

D3D12Renderer::D3D12Renderer()
{
	m_clientWidth = PER_DEFAULT_WINDOW_WIDTH;
	m_clientHeight = PER_DEFAULT_WINDOW_HEIGHT;

	m_factory = NULL;
	m_swapChain = NULL;
	m_device = NULL;

	m_commandQueue = NULL;
	m_commandAllocator = NULL;
	m_commandList = NULL;

	for (int i = 0; i < c_NUM_SWAP_CAHIN_BUFFERS; ++i) m_renderTargetBuffers[i] = NULL;
	m_rtvDescriptorHeap = NULL;
	m_rtvDescriptorIncrementSize = 0;

	m_depthStencilTargetBuffer = NULL;
	m_dsvDescriptorHeap = NULL;
	m_dsvDescriptorIncrementSize = 0;

	m_swapChainBufferIndex = 0;

	m_fence = NULL;
	for (int i = 0; i < c_NUM_SWAP_CAHIN_BUFFERS; ++i) m_fenceValues[i] = 0;
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
		if (m_renderTargetBuffers[i]) m_renderTargetBuffers[i]->Release();
	
	if (m_rtvDescriptorHeap) m_rtvDescriptorHeap->Release();
	if (m_depthStencilTargetBuffer) m_depthStencilTargetBuffer->Release();
	if (m_dsvDescriptorHeap) m_dsvDescriptorHeap->Release();

	if (m_commandAllocator) m_commandAllocator->Release();
	if (m_commandQueue) m_commandQueue->Release();
	if (m_commandList )m_commandList->Release();

	if (m_fence) m_fence->Release();

	if (m_swapChain) m_swapChain->Release();
	if (m_factory) m_factory->Release();
	if (m_device) m_device->Release();

#ifdef PER_DEBUG
	IDXGIDebug1* dxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&dxgiDebug);
	HRESULT result = dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	dxgiDebug->Release();
#endif 
	PERLog::Logger().Info("Direct3D 12 인테페이스 삭제 완료");
}

void D3D12Renderer::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL isFullScreenState = FALSE;
	m_swapChain->GetFullscreenState(&isFullScreenState, NULL);
	m_swapChain->SetFullscreenState(!isFullScreenState, NULL);

	DXGI_MODE_DESC targetParameters = d3d12_init::SetAndGetModeDesc(m_clientWidth, m_clientHeight);
	m_swapChain->ResizeTarget(&targetParameters);

	// 기존 렌더 타켓 뷰 삭제
	for (int i = 0; i < c_NUM_SWAP_CAHIN_BUFFERS; ++i)
	{
		if (m_renderTargetBuffers[i])m_renderTargetBuffers[i]->Release();
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	m_swapChain->GetDesc(&swapChainDesc);
	m_swapChain->ResizeBuffers(c_NUM_SWAP_CAHIN_BUFFERS, m_clientWidth, m_clientHeight, 
		swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);

	m_swapChainBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	// 리사이즈된 렌더 타켓 뷰 재생성
	CreateRenderTargetViews();
	CreateDepthStencilView();
}

void D3D12Renderer::MoveToNextFrame()
{
	m_swapChainBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	UINT64 fenceValue = ++m_fenceValues[m_swapChainBufferIndex];
	HRESULT result = m_commandQueue->Signal(m_fence, fenceValue);
	if (m_fence->GetCompletedValue() < fenceValue)
	{
		result = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void D3D12Renderer::BuildObjects(PERWorld* world)
{
	m_commandList->Reset(m_commandAllocator, NULL);

	// 게임 월드 내 게임 객체 생성
	world->BuildObjects(m_device, m_commandList);

	// 월드 객체를 생성하기 위해 필요한 그래픽 커맨드 큐를 커맨드 큐에 추가
	m_commandList->Close();
	ID3D12CommandList* commandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(1, commandLists);

	// 커맨드 리스트들이 모두 실행 될 때까지 대기
	WaitForGpuComplete();

	if (world)world->ReleaseUploadBuffers();
}

void D3D12Renderer::FrameAdvance(PERWorld* world, D3D12Camera* camera)
{
	// 커맨드 할당자, 커맨드 리스트 리셋
	HRESULT result = m_commandAllocator->Reset();
	result = m_commandList->Reset(m_commandAllocator, NULL);

	// 현재 렌더 타켓에 대한 프리젠트가 끝나길 대기
	// 프리젠트가 끝나면 렌더 타겟 버퍼의 상태는 프리젠트 상태에서 렌더 타겟 상태로 바뀜
	WaitForCurrentRenderTargetStateChange(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 뷰 클리어 및 출력 병합에 연결
	ClearViewAndSetToOM();

	// 렌더 코드 추가 위치
	world->Render(m_commandList, m_dsvDescriptorHeap, camera);

	// 현재 렌더 타겟이 렌더링 끝나길 대기
	// GPU가 렌더 타켓(버퍼)를 더 이상 사용하지 않으면 렌더 타겟의 상태는 프리젠트 상태로 바뀜
	WaitForCurrentRenderTargetStateChange(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// 커맨드 리스트를 닫힌 상태로 변경
	result = m_commandList->Close();

	// 커맨드 리스트를 커멘드 큐에 추가하여 실행
	ID3D12CommandList* commandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(1, commandLists);

	// GPU가 모든 커맨드 리스트를 실행할 때 까지 대기
	WaitForGpuComplete();

	// 스왑체인 프리젠트
	m_swapChain->Present(0, 0);

	// 다음 프레임으로 이동
	MoveToNextFrame();
}

void D3D12Renderer::SetClientSize(int width, int height)
{
	m_clientWidth = width;
	m_clientHeight = height;
}

void D3D12Renderer::WaitForGpuComplete()
{
	// CPU 펜스 값 증가
	UINT64 fenceValue = ++m_fenceValues[m_swapChainBufferIndex];

	// GPU가 펜스의 값을 설정하는 커맨드를 커맨드 큐에 추가
	HRESULT result = m_commandQueue->Signal(m_fence, fenceValue);
	if (m_fence->GetCompletedValue() < fenceValue)
	{
		result = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void D3D12Renderer::CreateDirect3DDevice()
{
	// 팩토리 생성
	m_factory = d3d12_init::CreateFactory();
	// 디바이스 생성
	m_device = d3d12_init::CreateDevice(m_factory);

	// MSAA 설정
	d3d12_init::SetMsaa(m_device, m_isMsaa4xEnable, m_msaa4xQualityLevels);
	// 펜스 생성
	m_fence = d3d12_init::CreateFence(m_device, m_fenceValues[0]);
	for (int i = 1; i < c_NUM_SWAP_CAHIN_BUFFERS; ++i) m_fenceValues[i] = m_fenceValues[0];

	// 펜스 이벤트 생성
	m_fenceEvent = d3d12_init::CreateFenceEvent();
}

void D3D12Renderer::CreateCommandQueueAndList()
{
	// 커맨드 관련 인터페이스 생성
	m_commandQueue = d3d12_init::CreateCommandQueue(m_device);
	m_commandAllocator = d3d12_init::CreateCommandAllocator(m_device);
	m_commandList = d3d12_init::CreateCommandList(m_device, m_commandAllocator);
}

void D3D12Renderer::CreateSwapchain(HWND hMainWnd)
{
	// 클라이언트 크기 받기
	d3d12_init::GetCleintSize(hMainWnd, m_clientWidth, m_clientHeight);

	// 스왑 체인 정보 설정 밎 받기
	DXGI_SWAP_CHAIN_DESC swapChainDesc = d3d12_init::SetAndGetSwapchainDesc(hMainWnd, m_clientWidth, m_clientHeight,
		m_isMsaa4xEnable, m_msaa4xQualityLevels, c_NUM_SWAP_CAHIN_BUFFERS);

	// 스왑 체인 생성
	m_swapChain = d3d12_init::CreateSwapchain(hMainWnd, m_factory, m_commandQueue,
		swapChainDesc, m_swapChainBufferIndex);
}

void D3D12Renderer::CreateRtvAndDsvDescriptorHeaps()
{
	// 렌더 타겟, 깊이-스텐실 디스크립터 힙 생성
	std::pair<ID3D12DescriptorHeap*, ID3D12DescriptorHeap*> heaps = d3d12_init::CreateDescriptorHeap(
		c_NUM_SWAP_CAHIN_BUFFERS, m_device, m_rtvDescriptorIncrementSize, m_dsvDescriptorIncrementSize);
	m_rtvDescriptorHeap = heaps.first;
	m_dsvDescriptorHeap = heaps.second;
}

void D3D12Renderer::CreateRenderTargetViews()
{
	// 렌더 타겟 버퍼, 뷰 생성
	for (UINT i = 0; i < c_NUM_SWAP_CAHIN_BUFFERS; ++i) {
		m_renderTargetBuffers[i] = d3d12_init::CreateRenerTargetView(m_swapChain, m_device,
			m_rtvDescriptorHeap, m_rtvDescriptorIncrementSize, i);
	}
}

void D3D12Renderer::CreateDepthStencilView()
{
	// 깊이-스텐실 버퍼, 뷰 생성
	m_depthStencilTargetBuffer = d3d12_init::CreateDepthStencilView(m_device, m_dsvDescriptorHeap,
		m_clientWidth, m_clientHeight, m_isMsaa4xEnable, m_msaa4xQualityLevels);
}

void D3D12Renderer::WaitForCurrentRenderTargetStateChange(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER resourceBarrier;
	::ZeroMemory(&resourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));

	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = m_renderTargetBuffers[m_swapChainBufferIndex];
	resourceBarrier.Transition.StateBefore = before;
	resourceBarrier.Transition.StateAfter = after;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_commandList->ResourceBarrier(1, &resourceBarrier);
}

void D3D12Renderer::ClearViewAndSetToOM()
{
	// 현재의 렌더 타겟에 해당하는 디스크립터의 CPU 주소(핸들) 얻기
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUDescriptorHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvCPUDescriptorHandle.ptr += m_swapChainBufferIndex * m_rtvDescriptorIncrementSize;

	// 원하는 색상으로 렌더 타켓(뷰) 클리어
	float clearColor[4] = { 0.f, 0.125f, 0.3f, 1.f }; // Colors::Azure
	m_commandList->ClearRenderTargetView(rtvCPUDescriptorHandle, clearColor, 0, NULL);

	// 깊이-스텐실 디스크립터의 CPU 주소 얻기
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUDescriptorHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// 원하는 값으로 깊이-스텐실(뷰) 클리어
	m_commandList->ClearDepthStencilView(dsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);

	// 렌더 타겟 뷰(디스크립터)와 깊이-스텐실 뷰(디스크립터)를 출력-병합 단계(OM)에 연결
	m_commandList->OMSetRenderTargets(1, &rtvCPUDescriptorHandle, TRUE, &dsvCPUDescriptorHandle);
}
