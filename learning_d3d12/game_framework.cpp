#include "stdafx.h"
#include "game_framework.h"

GameFramework::GameFramework()
{
	wcscpy_s(m_textFrameRate, L"D3DProject (");
}

GameFramework::~GameFramework()
{
	
}

bool GameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	// 렌더러 생성
	m_renderer.CreateInterface(hMainWnd);

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
	m_renderer.WaitForGpuComplete();

	// 스레드 종료
	EndWorkerThreads();	

	// 게임 객체 삭제
	ReleaseObjects();

	// 렌더러 삭제
	m_renderer.ReleaseInterface();

	PERLog::Logger().Info("게임 프레임워크 삭제 완료");
}

void GameFramework::BuildObjects()
{
}

void GameFramework::ReleaseObjects()
{
}

void GameFramework::Update(int deltaTime)
{
}

void GameFramework::Render()
{
	// 타이머 시간 갱신 및 프레임 레이트 계산
	m_timer.Tick();

	m_renderer.FrameAdvance();

	// 윈도우에 프레임 레이트 출력
	m_timer.GetFrameRate(m_textFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_textFrameRate);
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
		m_renderer.SetClientSize(LOWORD(lParam), HIWORD(lParam));
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
