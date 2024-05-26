#pragma once

class PERTimer {
public:
	PERTimer();
	virtual ~PERTimer();

	void Tick(float lockFPS = 0.0f);
	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int numCharacters = 0) const;
	float GetTimeElapsed() const;

private:
	void PerformanceCounterTick(float lockFPS = 0.0f);
	void NormalTick(float lockFPS = 0.0f);
	void CalculateFPS(float timeElapsed);

	static const int c_MAX_SAMPLE_COUNT = 60;

	std::function<void(PERTimer&, float)> m_TickFunc;

	bool	m_isHardwareHasPerformanceCounter;
	float	m_timeScale;
	float	m_timeElapsed;
	__int64 m_currentTime;
	__int64 m_lastTime;
	__int64 m_performanceFrequency;

	float	m_frameTime[c_MAX_SAMPLE_COUNT];
	ULONG	m_sampleCount;

	unsigned long	m_currentFrameRate;
	unsigned long	m_framesPerSecond;
	float			m_fpsTimeElapsed;
};