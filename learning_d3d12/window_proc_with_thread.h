#pragma once
#include "stdafx.h"
#include "console_logger.h"

inline int CalculateDeltaTime(std::chrono::system_clock::time_point* lastTime, std::chrono::system_clock::time_point* currentTime);
inline void SleepForRestDevice(int dTime);

void CreateWorkerThreads(DWORD threadID);

DWORD WINAPI GameTheadFunc(LPVOID temp);
DWORD WINAPI LogTheadFunc(LPVOID temp);