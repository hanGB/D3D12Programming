#include "stdafx.h"
#include "simple_timer.h"

SimpleTimer::SimpleTimer()
{
	m_lastTime = std::chrono::system_clock::now();
}

SimpleTimer::~SimpleTimer()
{
}

int SimpleTimer::CalculateDeltaTime()
{
	// 현재 시간 측정
	m_currentTime = std::chrono::system_clock::now();
	auto deltaTime = m_currentTime - m_lastTime;

	// 경과된 시간(마이크로초 단위)
	int dTime = (int)std::chrono::duration_cast<std::chrono::microseconds>(deltaTime).count();

	// 현재 시간을 마지막 시간으로 저장
	m_lastTime = m_currentTime;

	return dTime;
}

void SimpleTimer::SleepForRestDevice(int dTime)
{
	// 너무 빠를 경우 휴식
	int restTime = PER_MINIMUM_FRAME_TIME - dTime;
	if (restTime > 0) {
		Sleep(int(restTime / 1'000.0));
	}
}

