#include "stdafx.h"
#include "game_framework.h"

DWORD __stdcall GameFramework::WrapGameThread(LPVOID lParam)
{
	GameFramework* framework = (GameFramework*)lParam;
	framework->GameTheadFunc();
	return 0;
}

DWORD __stdcall GameFramework::WrapLogThread(LPVOID lParam)
{
	GameFramework* framework = (GameFramework*)lParam;
	framework->LogTheadFunc();
	return 0;
}

void GameFramework::GameTheadFunc()
{
	PERLog::Logger().Info("게임 스레드 시작");

	int dTime;
	auto lastTime = std::chrono::system_clock::now(); auto currentTime = std::chrono::system_clock::now();
	while (!m_isGameEnd) {
		dTime = CalculateDeltaTime(&lastTime, &currentTime);
		Update(dTime);
		SleepForRestDevice(dTime);
	}

	PERLog::Logger().Info("게임 스레드 종료");
}

void GameFramework::LogTheadFunc()
{
	PERLog::Logger().Info("로그 스레드 시작");

	int dTime;
	auto lastTime = std::chrono::system_clock::now(); auto currentTime = std::chrono::system_clock::now();
	while (!m_isGameEnd) {
		dTime = CalculateDeltaTime(&lastTime, &currentTime);
		PERLog::Logger().Update();
		SleepForRestDevice(dTime);
	}

	PERLog::Logger().Info("로그 스레드 종료");
}