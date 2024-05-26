#include "stdafx.h"
#include "per_timer.h"

PERTimer::PERTimer()
{
	if (::QueryPerformanceFrequency((LARGE_INTEGER*)&m_performanceFrequency))
	{
		m_TickFunc = &PERTimer::PerformanceCounterTick;
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_lastTime);
		m_timeScale = 1.0f / m_performanceFrequency;
	}
	else
	{
		m_TickFunc = &PERTimer::NormalTick;
		m_lastTime = ::timeGetTime();
		m_timeScale = 0.001f;
	}

	m_sampleCount = 0;
	m_currentFrameRate = 0;
	m_framesPerSecond = 0;
	m_fpsTimeElapsed = 0.0f;
}

PERTimer::~PERTimer()
{
}

void PERTimer::Tick(float lockFPS)
{
	m_TickFunc(*this, lockFPS);
}

unsigned long PERTimer::GetFrameRate(LPTSTR lpszString, int numCharacters) const
{
	if (lpszString)
	{
		_itow_s(m_currentFrameRate, lpszString, numCharacters, 10);
		wcscat_s(lpszString, numCharacters, L" FPS)");
	}

	return m_currentFrameRate;
}

float PERTimer::GetTimeElapsed() const
{
	return m_timeElapsed;
}

void PERTimer::PerformanceCounterTick(float lockFPS)
{
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentTime);
	float timeElaped = (m_currentTime - m_lastTime) * m_timeScale;

	// lockFPS 만큼 대기
	if (lockFPS > 0.0f)
	{
		while (timeElaped < (1.0f / lockFPS))
		{
			::QueryPerformanceCounter((LARGE_INTEGER*)&m_currentTime);
			timeElaped = (m_currentTime - m_lastTime) * m_timeScale;
		}
	}
	m_lastTime = m_currentTime;
	CalculateFPS(timeElaped);
}

void PERTimer::NormalTick(float lockFPS)
{
	m_currentTime = ::timeGetTime();
	float timeElaped = (m_currentTime - m_lastTime) * m_timeScale;

	// lockFPS 만큼 대기
	if (lockFPS > 0.0f)
	{
		while (timeElaped < (1.0f / lockFPS))
		{
			m_currentTime = ::timeGetTime();
			timeElaped = (m_currentTime - m_lastTime) * m_timeScale;
		}
	}
	m_lastTime = m_currentTime;
	CalculateFPS(timeElaped);
}

void PERTimer::CalculateFPS(float timeElapsed)
{
	// 마지막 프레임 시간과 현재 프레임 시간 차이가 1초보다 작을 경우 현재 프레임 시간을 m_frameTime[0]에 저장
	if (fabsf(timeElapsed - m_timeElapsed) < 1.0f)
	{
		::memmove(&m_frameTime[1], m_frameTime, (c_MAX_SAMPLE_COUNT - 1) * sizeof(float));
		m_frameTime[0] = timeElapsed;
		if (m_sampleCount < c_MAX_SAMPLE_COUNT) m_sampleCount++;
	}

	// 초당 프레임 수를 1 증가시키고 현재 프레임 처리 시간을 누적하여 저장
	m_framesPerSecond++;
	m_fpsTimeElapsed += timeElapsed;
	if (m_fpsTimeElapsed > 1.0f)
	{
		m_currentFrameRate = m_framesPerSecond;
		m_framesPerSecond = 0;
		m_fpsTimeElapsed = 0.0f;
	}

	// 누적된 프레임 처리 시간의 평균 계산
	m_timeElapsed = 0.0f;
	for (ULONG i = 0; i < m_sampleCount; ++i) m_timeElapsed += m_frameTime[i];
	if (m_sampleCount > 0) m_timeElapsed /= m_sampleCount;
}
