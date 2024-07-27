#include "stdafx.h"
#include "game_timer.h"

GameTimer::GameTimer()
	: m_secondsPerCount(0.0), m_deltaTime(-1.0), m_baseTime(0),
	m_pausedTime(0), m_currentTime(0), m_isStoped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.0 / (double)countsPerSec;
}

float GameTimer::TotalTime() const
{
	// 멈췄던 시간을 제외하고 베이스 타임에서 얼마나 지났는 지 계산
	if (m_isStoped)
	{
		return (float)(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
	else
	{
		return (float)(((m_currentTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
}

float GameTimer::DeltaTime() const
{
	return (float)m_deltaTime;
}

void GameTimer::Reset()
{
	// 현재 시간으로 리셋
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	m_baseTime = currentTime;
	m_prevTime = currentTime;
	m_stopTime = 0;
	m_isStoped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (m_isStoped)
	{
		// 일시 정지된 시간 누적
		m_pausedTime += (startTime - m_stopTime);

		// 이전 시간 다시 설정
		m_prevTime = startTime;

		// 정지 시간 초기화
		m_stopTime = 0;
		m_isStoped = false;
	}
}

void GameTimer::Stop()
{
	if (!m_isStoped)
	{
		__int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		// 현재 시간을 타이머 정지 시점으로 설정
		m_stopTime = currentTime;
		m_isStoped = true;
	}
}

void GameTimer::Tick()
{
	if (m_isStoped)
	{
		m_deltaTime = 0.0;
		return;
	}

	// 이번 프레임의 시간 얻음
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	m_currentTime = currentTime;

	// 이번 프레임의 시간과 이전 프ㅔ임의 시간 차이 계산
	m_deltaTime = (m_currentTime - m_prevTime) * m_secondsPerCount;

	// 다음 프레임 준비
	m_prevTime = m_currentTime;

	// 음수가 되지 않게 설정
	if (m_deltaTime < 0.0) m_deltaTime = 0.0;
}
