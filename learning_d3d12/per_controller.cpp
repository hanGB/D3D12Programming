#include "stdafx.h"
#include "per_controller.h"

PERController::PERController(int width, int height)
{
	// 초기화
	for (int i = 0; i < (int)PERKeyValue::NUM_KEY_VALUE; ++i) {
		m_KeyPressedMap.insert(std::pair<PERKeyValue, bool>((PERKeyValue)i, false));
		m_KeyProcessedMap.insert(std::pair<PERKeyValue, bool>((PERKeyValue)i, false));
		m_KeyInputTimeMap.insert(std::pair<PERKeyValue, float>((PERKeyValue)i, 0.0));
	}

	GiveMouseMoveInput((double)width / 2.0, (double)height / 2.0);
	SetWindowSize((double)width, (double)height);
}

PERController::~PERController()
{	
}

void PERController::GiveKeyInput(PERKeyValue key, bool pressed)
{
	if (m_KeyPressedMap.find(key)->second == pressed) return;

	m_KeyPressedMap.find(key)->second = pressed;
	InitProcessedAndInputTime(key, false);
}

void PERController::GiveMouseMoveInput(double x, double y)
{
	m_mousePosX = x;
	m_mousePosY = y;
}

bool PERController::GetIsMouseFixed() const
{
	return m_isMouseFixed;
}

void PERController::SetIsMouseFixed(bool fix)
{
	m_isMouseFixed = fix;
}

void PERController::GetMousePos(double* x, double* y)
{
	*x = m_mousePosX;
	*y = m_mousePosY;
}

void PERController::SetWindowSize(double width, double height)
{
	m_windowWidth = width;
	m_windowHeight = height;
}

void PERController::GetWindowSize(double* width, double* height)
{
	*width = m_windowWidth;
	*height = m_windowHeight;
}

bool PERController::IsInputed(PERKeyValue key, bool pressed)
{
	if (m_KeyPressedMap.find(key)->second != pressed) return false;

	InitProcessedAndInputTime(key, true);

	return true;
}

bool PERController::IsInputedNotSetProcessed(PERKeyValue key, bool pressed)
{
	return (m_KeyPressedMap.find(key)->second == pressed);
}

bool PERController::IsInputedNotProcessed(PERKeyValue key, bool pressed)
{
	if (!IsInputedNotSetProcessed(key, pressed)) return false;
	if (m_KeyProcessedMap.find(key)->second == true) return false;

	m_KeyProcessedMap.find(key)->second = true;

	return true;
}

bool PERController::IsInputedMoreThanTime(PERKeyValue key, bool pressed, float time)
{
	if (!IsInputedNotSetProcessed(key, pressed)) return false;
	if (m_KeyInputTimeMap.find(key)->second < time) return false;
	m_KeyInputTimeMap.find(key)->second = 0.0;
	return true;
}

bool PERController::IsInputedNotProcessedOrMoreThanTime(PERKeyValue key, bool pressed, float time)
{
	if (!IsInputedNotSetProcessed(key, pressed)) return false;

	if (m_KeyProcessedMap.find(key)->second == false)
	{
		InitProcessedAndInputTime(key, true);
		return true;
	}
	if (m_KeyInputTimeMap.find(key)->second > time)
	{
		m_KeyInputTimeMap.find(key)->second = 0.0;
		return true;
	}

	return false;
}

void PERController::Update(float dTime)
{
	for (auto& it : m_KeyInputTimeMap) {
		it.second += dTime;
	}
}

void PERController::InitProcessedAndInputTime(PERKeyValue key, bool processed)
{
	m_KeyProcessedMap.find(key)->second = processed;
	m_KeyInputTimeMap.find(key)->second = 0.0;
}
