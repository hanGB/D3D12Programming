#include "stdafx.h"
#include "game_framework.h"

GameFramework::GameFramework()
{

}

GameFramework::~GameFramework()
{
}

bool GameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	// 렌더러 생성
	m_renderer = new D3D12Renderer();
	m_renderer->CreateInterface(hMainWnd);

	// 게임 객체 생성
	BuildObjects();

	// 쓰레드 생성
	CreateWorkerThreads();

	PERLog::Logger().Info("게임 프레임워크 생성 완료");

	return true;
}

void GameFramework::OnDestroy()
{
	// GPU가 모든 명령 리스트 실행 완료할 때까지 대기
	m_renderer->WaitForGpuComplete();

	// 스레드 종료
	EndWorkerThreads();	

	// 게임 객체 삭제
	ReleaseObjects();

	// 렌더러 삭제
	m_renderer->ReleaseInterface();
	delete m_renderer;

	PERLog::Logger().Info("게임 프레임워크 삭제 완료");
}

void GameFramework::BuildObjects()
{
}

void GameFramework::ReleaseObjects()
{
}

void GameFramework::Update(float deltaTime)
{
}

void GameFramework::Render()
{
	m_renderer->FrameAdvance();
}

void GameFramework::CreateWorkerThreads()
{
	m_isGameEnd = false;

	m_hWorkerThreads[0] = CreateThread(NULL, 0, WrapGameThread, (void*)this, 0, &m_threadID);
	m_hWorkerThreads[1] = CreateThread(NULL, 0, WrapLogThread, (void*)this, 0, &m_threadID);
}

void GameFramework::EndWorkerThreads()
{
	m_isGameEnd = true;

	WaitForMultipleObjects(c_NUM_WORKER_THREAD, m_hWorkerThreads, true, INFINITE);
}

void GameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageId, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageId)
	{
	case WM_LBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		break;
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void GameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageId, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageId)
	{
	case WM_KEYUP:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		}
	}
	default:
		break;
	}
}

LRESULT GameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageId, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageId)
	{
	case WM_SIZE: {
		m_renderer->SetClientSize(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageId, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageId, wParam, lParam);
		break;
	}

	return 0;
}


int GameFramework::CalculateDeltaTime(std::chrono::system_clock::time_point* lastTime, std::chrono::system_clock::time_point* currentTime)
{
	// 현재 시간 측정
	*currentTime = std::chrono::system_clock::now();
	auto deltaTime = *currentTime - *lastTime;

	// 경과된 시간(마이크로초 단위)
	int dTime = (int)std::chrono::duration_cast<std::chrono::microseconds>(deltaTime).count();

	// 현재 시간을 마지막 시간으로 저장
	*lastTime = *currentTime;

	return dTime;
}

void GameFramework::SleepForRestDevice(int dTime)
{
	// 너무 빠를 경우 휴식
	int restTime = PER_MINIMUM_FRAME_TIME - dTime;
	if (restTime > 0) {
		Sleep(int(restTime / 1'000.0));
	}
}
