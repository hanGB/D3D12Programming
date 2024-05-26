#include "stdafx.h"
#include "game_framework.h"
#include "simple_timer.h"

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

	SimpleTimer timer;
	while (!m_isGameEnd) {
		int dTime = timer.CalculateDeltaTime();
		Update(dTime);
		timer.SleepForRestDevice(dTime);
	}

	PERLog::Logger().Info("게임 스레드 종료");
}

void GameFramework::LogTheadFunc()
{
	PERLog::Logger().Info("로그 스레드 시작");

	SimpleTimer timer;
	while (!m_isGameEnd) {
		int dTime = timer.CalculateDeltaTime();
		PERLog::Logger().Update();
		timer.SleepForRestDevice(dTime);
	}

	PERLog::Logger().Info("로그 스레드 종료");
}
