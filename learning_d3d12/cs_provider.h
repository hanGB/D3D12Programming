#pragma once
#include "stdafx.h"

class CSProvider {
public:
	CSProvider()
	{
		InitializeCriticalSection(&m_cs);
	}
	~CSProvider()
	{
		DeleteCriticalSection(&m_cs);
	}

	void Lock()
	{
		EnterCriticalSection(&m_cs);
	}
	void Unlock()
	{
		LeaveCriticalSection(&m_cs);
	}

private:
	CRITICAL_SECTION m_cs;
};