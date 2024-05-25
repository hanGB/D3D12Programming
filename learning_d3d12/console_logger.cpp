#include "stdafx.h"
#include "console_logger.h"

ConsoleLogger::ConsoleLogger()
{
	AllocConsole();

	FILE* stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stderr);
	freopen_s(&stream, "CONOUT$", "w", stdout);

	// 콘솔 UTF-8로 설정
	SetConsoleOutputCP(65001);

	Info("콘솔 로거 생성");
}

ConsoleLogger::~ConsoleLogger()
{
	Info("콘솔 로거 삭제");

	FreeConsole();
}

void ConsoleLogger::Update()
{
	m_csProvider.Lock();

	if (m_head == m_tail) {
		m_csProvider.Unlock();
		return;
	}

	std::cout << m_timeOutputbuffer[m_head] << "\n";
	std::cout << m_textOutputbuffer[m_head] << "\n";
	std::cout << "---------------------------------------------------" << "\n";
	m_head = (m_head + 1) % m_maxPending;

	m_csProvider.Unlock();
}

void ConsoleLogger::Info(const char* text)
{
	m_csProvider.Lock();

	if ((m_tail + 1) % m_maxPending == m_head) {
		m_csProvider.Unlock();
		return;
	}

	sprintf_s(m_textBuffer, m_textBufferSize, "(INFO)%s", text);
	memmove_s(m_textOutputbuffer[m_tail], c_LOG_BUFFER_SIZE, m_textBuffer, m_textBufferSize);
	memmove_s(m_timeOutputbuffer[m_tail], c_LOG_BUFFER_SIZE, GetNowTime(), m_timeBufferSize);

	m_tail = (m_tail + 1) % m_maxPending;

	m_csProvider.Unlock();
}

void ConsoleLogger::Warnning(const char* text)
{
	m_csProvider.Lock();

	if ((m_tail + 1) % m_maxPending == m_head) {
		m_csProvider.Unlock();
		return;
	}

	sprintf_s(m_textBuffer, m_textBufferSize, "(WARN)%s", text);
	memmove_s(m_textOutputbuffer[m_tail], c_LOG_BUFFER_SIZE, m_textBuffer, m_textBufferSize);
	memmove_s(m_timeOutputbuffer[m_tail], c_LOG_BUFFER_SIZE, GetNowTime(), m_timeBufferSize);

	m_tail = (m_tail + 1) % m_maxPending;

	m_csProvider.Unlock();
}

void ConsoleLogger::Error(const char* text)
{
	m_csProvider.Lock();

	if ((m_tail + 1) % m_maxPending == m_head) {
		m_csProvider.Unlock();
		return;
	}

	sprintf_s(m_textBuffer, m_textBufferSize, "(ERRO)%s", text);
	memmove_s(m_textOutputbuffer[m_tail], c_LOG_BUFFER_SIZE, m_textBuffer, m_textBufferSize);
	memmove_s(m_timeOutputbuffer[m_tail], c_LOG_BUFFER_SIZE, GetNowTime(), m_timeBufferSize);

	m_tail = (m_tail + 1) % m_maxPending;

	m_csProvider.Unlock();
}


void ConsoleLogger::SaveLogs()
{
}

void ConsoleLogger::SaveWarnnings()
{
}

void ConsoleLogger::SaveErrors()
{
}

void ConsoleLogger::SaveAll()
{
}
