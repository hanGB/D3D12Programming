#include "stdafx.h"
#include "shapes_app.h"

ShapesApp::ShapesApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

ShapesApp::~ShapesApp()
{
}

bool ShapesApp::Initialize()
{
	BuildFrameResources();

	return true;
}

void ShapesApp::OnResize()
{
}

void ShapesApp::Update(const GameTimer& gt)
{
	// 다음 프레임 리소스로 설정
	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % c_NUM_FRAME_RESOURCES;

	// GPU가 현재 프레임 리소스의 커맨드들을 다 처리했는지 확인
	if (m_currentFrameResource->fence != 0 && m_fence->GetCompletedValue() < m_currentFrameResource->fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFrameResource->fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	// 현재 프레임 리소스 갱신

}

void ShapesApp::Draw(const GameTimer& gt)
{
	// 이 프레임의 커맨드 리스트를 구축하고 제출


	// 현재 펜스 지점까지의 커맨드들을 표시하도록 펜스 값을 전진
	m_currentFrameResource->fence = ++m_currentFence;

	// 새 펜스 지점을 설정하는 커맨드를 커맨드 큐에 추가
	m_commandQueue->Signal(m_fence.Get(), m_currentFence);

	// GPU가 아직 이전 프레임들의 명령을 처리하고 있지만 관련 리소스를 건들지 않기 때문에 문제 없음
}

void ShapesApp::BuildFrameResources()
{
	for (int i = 0; i < c_NUM_FRAME_RESOURCES; ++i)
	{
		// m_frameResource.push_back(std::make_unique<ShapesFrameResource>(m_d3dDevice, 1, (UINT)m_allRenderItems.size()));
	}
}
