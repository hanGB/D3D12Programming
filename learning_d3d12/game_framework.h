#pragma once
#include "d3d12_renderer.h"
#include "per_timer.h"
#include "per_world.h"

class GameFramework {
public: 
	GameFramework();
	~GameFramework();
	
	// 생성 삭제
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void ChangeScreenMode();

	// 월드 생성, 삭제
	void CreateWorld();
	void DeleteWorld();

	// 업데이트, 렌더링
	void Update(int deltaTime);
	void Render();

	// 멀티 스레드
	void CreateWorkerThreads();
	void EndWorkerThreads();

	// 윈도우 메세지(마우스, 키보드 입력) 처리
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageId, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageId, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageId, WPARAM wParam, LPARAM lParam);

private:
	// 스레드로 실행되는 함수
	static DWORD WINAPI WrapGameThread(LPVOID lParam);
	static DWORD WINAPI WrapLogThread(LPVOID lParam);
	void GameTheadFunc();
	void LogTheadFunc();

	static const int c_NUM_WORKER_THREAD = 2;

	HINSTANCE m_hInstance;
	HWND m_hWnd;

	// 멀티 스레드
	HANDLE m_hWorkerThreads[c_NUM_WORKER_THREAD];
	DWORD m_threadID;
	bool m_isGameEnd;

	// 클래스
	D3D12Renderer m_renderer;
	PERTimer m_timer;

	// 월드
	PERWorld* m_world;

	wchar_t m_textFrameRate[50];
};