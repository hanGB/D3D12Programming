#include "stdafx.h"
#include "init_direct3d_app.h"

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

InitDirect3DApp::~InitDirect3DApp()
{
}

bool InitDirect3DApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	return true;
}

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();
}

void InitDirect3DApp::Update(const GameTimer& gt)
{

}

void InitDirect3DApp::Draw(const GameTimer& gt)
{
	// 커맨드 레코드 관련 메모리를 재활용을 위해 커맨드 할당자 재설정
	// GPU가 관련 커맨드 리스트를 모두 처리한 후 재설정됨
	ThrowIfFailed(m_commandListAllocator->Reset());

	// 커맨드 리스트를 ExcuteCommandList를 통해서 커맨드 큐에 추가했다면 커맨드 리스트를 재설정 가능
	// 커맨드 리스트 재설정 시 메모리 재활용
	ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

	// 리소스 용도에 관련된 상태 전이를 Direct3D에 통지(Prsent -> Render Target)
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 뷰포트와 씨져 사각형 설정
	m_commandList->RSSetViewports(1, &m_screenViewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// 후면 버퍼와 깊이 버퍼 클리어
	m_commandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 렌더링 결과가 기록될 렌더 대상 버퍼 지정
	m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// 리소스 용도에 관련된 상태 전이를 Direct3D에 통지(Render Target -> Present)
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// 커맨드 레코드 종료
	ThrowIfFailed(m_commandList->Close());

	// 커맨드 실행을 위한 커맨드 리스트를 커맨드 큐에 추가
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// 후면 버퍼와 전면 버퍼 교환
	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_currentBackBuffer = (m_currentBackBuffer + 1) % c_SWAP_CHAIN_BUFFER_COUNT;

	// 이 프레임의 커맨드들이 모두 처리되길 대기
	// 프레임마다 대기하는 것은 비효율적이므로 할 필요없도록 만들어야 됨
	FlushCommandQueue();
}
