#pragma once
#include "cs_provider.h"

class PERLogger {
public:
	virtual ~PERLogger() {}

	virtual void Update() = 0;

	virtual void Info(const char* text) = 0;
	virtual void Warnning(const char* text) = 0;
	virtual void Error(const char* text) = 0;

	template<class... Args>
	void InfoWithFormat(const char* format, Args... args) {
		sprintf_s(m_buffer, m_bufferSize, format, args...);
		Info(m_buffer);
	}
	template<class... Args>
	void WarnningWithFormat(const char* format, Args... args) {
		sprintf_s(m_buffer, m_bufferSize, format, args...);
		Warnning(m_buffer);
	}
	template<class... Args>
	void ErrorWithFormat(const char* format, Args... args) {
		sprintf_s(m_buffer, m_bufferSize, format, args...);
		Error(m_buffer);
	}

protected:
	const char* GetNowTime() {
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		ctime_s(m_timeBuffer, m_timeBufferSize, &time);

		return m_timeBuffer;
	}

	static const int c_LOG_BUFFER_SIZE = 1024;
	static const int c_LOG_WAIT_SIZE = 1000;

	char m_buffer[c_LOG_BUFFER_SIZE];
	int m_bufferSize = c_LOG_BUFFER_SIZE;

	char m_textBuffer[c_LOG_BUFFER_SIZE];
	int m_textBufferSize = c_LOG_BUFFER_SIZE;

	char m_timeBuffer[c_LOG_BUFFER_SIZE];
	int m_timeBufferSize = c_LOG_BUFFER_SIZE;

	CSProvider m_csProvider;

	//  
	char m_textOutputbuffer[c_LOG_WAIT_SIZE][c_LOG_BUFFER_SIZE];
	char m_timeOutputbuffer[c_LOG_WAIT_SIZE][c_LOG_BUFFER_SIZE];

	int m_head = 0, m_tail = 0;
	int m_maxPending = c_LOG_WAIT_SIZE;
};