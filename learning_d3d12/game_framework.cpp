#include "stdafx.h"
#include "game_framework.h"
#include "camera_component.h"

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

	// 오브젝트 생성
	CreateObjectsWithRenderer();

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

	// 오브젝트들 삭제
	DeleteObjects();

	// 렌더러 삭제
	m_renderer.ReleaseInterface();

	PERLog::Logger().Info("게임 프레임워크 삭제 완료");
}

void GameFramework::ChangeScreenMode()
{
	m_renderer.ChangeSwapChainState();
}

void GameFramework::CreateObjectsWithRenderer()
{
	m_player = new PERPlayer();
	m_world = new PERWorld();
	m_renderer.BuildObjects(m_world, m_player);
	m_camera = m_player->GetComponentWithType<CameraComponent>()->GetCamera();
}

void GameFramework::DeleteObjects()
{
	if (m_camera) m_camera->ReleaseShderVariables();
	if (m_camera) delete m_camera;
	if (m_world) m_world->ReleaseObjects();
	if (m_world) delete m_world;
}

void GameFramework::Update(int deltaTime)
{
	float dTime = (float)deltaTime / 1'000'000.0f;

	m_world->InputUpdate(m_controller, dTime);
	m_player->GetInput().Update(m_controller, dTime);
	m_world->AiUpdate(dTime);
	m_world->PhysicsUpdate(dTime);
	m_player->GetPhysics().Update(dTime);
	if (m_updateEnd) return;

	m_world->GraphicsUpdate(dTime);
	m_player->GetGraphics().Update(dTime);
	m_updateEnd = true;
}

void GameFramework::Render()
{
	if (!m_updateEnd) return;

	// 타이머 시간 갱신 및 프레임 레이트 계산
	m_timer.Tick();

	m_renderer.FrameAdvance(m_world, m_player, m_camera);

	// 윈도우에 프레임 레이트 출력
	m_timer.GetFrameRate(m_textFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_textFrameRate);

	m_updateEnd = false;
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
		case VK_F1:
		case VK_F2:
		case VK_F3:
			if (m_player) m_camera = m_player->GetComponentWithType<CameraComponent>()->ChangeCamera(wParam - VK_F1 + 1, 0.0f);
			break;
		case VK_F9:
			ChangeScreenMode();
			break;
		}
		break;
	
	}
	default:
		break;
	}

	// 컨트롤러 입력
	switch (nMessageId)
	{
	case WM_KEYUP: 
		m_controller.HandleWindowKeyboardInput(wParam, false);
		break;
	case WM_KEYDOWN:
		m_controller.HandleWindowKeyboardInput(wParam, true);
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
