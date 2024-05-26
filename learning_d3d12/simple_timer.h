#pragma once

class SimpleTimer {
public:
	SimpleTimer();
	~SimpleTimer();

	// 시간 처리
	int CalculateDeltaTime();
	void SleepForRestDevice(int dTime);

private:
	std::chrono::system_clock::time_point m_lastTime;
	std::chrono::system_clock::time_point m_currentTime;
};