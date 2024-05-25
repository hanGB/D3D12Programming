#include "stdafx.h"
#include "window_proc_with_thread.h"

HINSTANCE g_hInst;
HWND g_hWnd;
HANDLE g_hWorkerThreads[PER_NUM_WORKER_THREAD];
bool g_isGameEnd;

inline int CalculateDeltaTime(std::chrono::system_clock::time_point* lastTime, std::chrono::system_clock::time_point* currentTime)
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

inline void SleepForRestDevice(int dTime)
{
	// 너무 빠를 경우 휴식
	int restTime = PER_MINIMUM_FRAME_TIME - dTime;
	if (restTime > 0) {
		Sleep(int(restTime / 1'000.0));
	}
}

void CreateWorkerThreads(DWORD threadID)
{
	g_hWorkerThreads[0] = CreateThread(NULL, 0, GameTheadFunc, NULL, 0, &threadID);
	g_hWorkerThreads[1] = CreateThread(NULL, 0, LogTheadFunc, NULL, 0, &threadID);
}

DWORD WINAPI GameTheadFunc(LPVOID temp)
{
	PERLog::Logger().Info("게임 스레드 시작");

	int dTime;
	auto lastTime = std::chrono::system_clock::now(); auto currentTime = std::chrono::system_clock::now();
	while (!g_isGameEnd) {
		dTime = CalculateDeltaTime(&lastTime, &currentTime);

		SleepForRestDevice(dTime);
	}

	PERLog::Logger().Info("게임 스레드 종료");
	return 0;
}

DWORD WINAPI LogTheadFunc(LPVOID temp)
{
	PERLog::Logger().Info("로그 스레드 시작");

	int dTime;
	auto lastTime = std::chrono::system_clock::now(); auto currentTime = std::chrono::system_clock::now();
	while (!g_isGameEnd) {
		dTime = CalculateDeltaTime(&lastTime, &currentTime);
		PERLog::Logger().Update();
		SleepForRestDevice(dTime);
	}

	PERLog::Logger().Info("로그 스레드 종료");
	return 0;
}