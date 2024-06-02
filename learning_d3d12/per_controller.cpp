#include "stdafx.h"
#include "per_controller.h"

PERController::PERController()
{
	// 초기화
	for (int i = 0; i < (int)PERKeyValue::NUM_KEY_VALUE; ++i) {
		m_KeyPressedMap.insert(std::pair<PERKeyValue, bool>((PERKeyValue)i, false));
		m_KeyProcessedMap.insert(std::pair<PERKeyValue, bool>((PERKeyValue)i, false));
		m_KeyInputTimeMap.insert(std::pair<PERKeyValue, float>((PERKeyValue)i, 0.0));
	}

	GiveMouseMoveInput((float)PER_DEFAULT_WINDOW_WIDTH / 2.0, (float)PER_DEFAULT_WINDOW_HEIGHT / 2.0);
	SetWindowSize((float)PER_DEFAULT_WINDOW_WIDTH, (float)PER_DEFAULT_WINDOW_WIDTH);
}

PERController::~PERController()
{	
}

void PERController::HandleWindowKeyboardInput(WPARAM wParam, bool pressed)
{
	if (wParam == 'W' || wParam == 'w') GiveKeyInput(PERKeyValue::W, pressed);
	if (wParam == 'A' || wParam == 'a') GiveKeyInput(PERKeyValue::A, pressed);
	if (wParam == 'S' || wParam == 's') GiveKeyInput(PERKeyValue::S, pressed);
	if (wParam == 'D' || wParam == 'd') GiveKeyInput(PERKeyValue::D, pressed);
	if (wParam == 'Q' || wParam == 'q') GiveKeyInput(PERKeyValue::Q, pressed);
	if (wParam == 'E' || wParam == 'e') GiveKeyInput(PERKeyValue::E, pressed);
	if (wParam == 'R' || wParam == 'r') GiveKeyInput(PERKeyValue::R, pressed);
	if (wParam == 'F' || wParam == 'f') GiveKeyInput(PERKeyValue::F, pressed);
	if (wParam == 'C' || wParam == 'c') GiveKeyInput(PERKeyValue::C, pressed);
	if (wParam == 'V' || wParam == 'v') GiveKeyInput(PERKeyValue::V, pressed);
}

void PERController::GiveKeyInput(PERKeyValue key, bool pressed)
{
	if (m_KeyPressedMap.find(key)->second == pressed) return;

	m_KeyPressedMap.find(key)->second = pressed;
	InitProcessedAndInputTime(key, false);
}

void PERController::GiveMouseMoveInput(float x, float y)
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

void PERController::GetMousePos(float* x, float* y)
{
	*x = m_mousePosX;
	*y = m_mousePosY;
}

void PERController::SetWindowSize(float width, float height)
{
	m_windowWidth = width;
	m_windowHeight = height;
}

void PERController::GetWindowSize(float* width, float* height)
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
