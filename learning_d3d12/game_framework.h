#pragma once
#include "d3d12_renderer.h"

class GameFramework {
public: 
	GameFramework();
	~GameFramework();
	
	// 생성 삭제
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	// 오브젝트 생성 삭제
	void BuildObjects();
	void ReleaseObjects();

	// 업데이트, 렌더링
	void Update(float deltaTime);
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

	// 시간 처리
	int CalculateDeltaTime(std::chrono::system_clock::time_point* lastTime, std::chrono::system_clock::time_point* currentTime);
	void SleepForRestDevice(int dTime);


	static const int c_NUM_WORKER_THREAD = 2;

	HINSTANCE m_hInstance;
	HWND m_hWnd;

	// 멀티 스레드
	HANDLE m_hWorkerThreads[c_NUM_WORKER_THREAD];
	DWORD m_threadID;
	bool m_isGameEnd;

	D3D12Renderer* m_renderer;
};