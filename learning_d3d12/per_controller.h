#pragma once
#include <atomic>
#include <concurrent_unordered_map.h>
#include "key_setting.h"
/*
	플레이어의 인풋을 받아 넘겨 주는 클래스
*/

class PERController {
public:
	PERController();
	~PERController();

	void HandleWindowKeyboardInput(WPARAM wParam, bool pressed);

	// 키 입력 저장
	void GiveKeyInput(PERKeyValue key, bool pressed);
	void GiveMouseMoveInput(float x, float y);

	bool GetIsMouseFixed() const;
	void SetIsMouseFixed(bool fix);
	void GetMousePos(float* x, float* y);

	void SetWindowSize(float width, float height);
	void GetWindowSize(float* width, float* height);

	// 키 입력 확인
	bool IsInputed(PERKeyValue key, bool pressed = true);
	// 키 입력을 처리하지만 처리되었다고 설정하지는 않음
	bool IsInputedNotSetProcessed(PERKeyValue key, bool pressed = true);
	// 키가 입력되고 처리된 적 없는 지 확인
	bool IsInputedNotProcessed(PERKeyValue key, bool pressed = true);
	// 키가 입력되고 시간이 충분히 지났을 경우
	bool IsInputedMoreThanTime(PERKeyValue key, bool pressed = true, float time = PER_DEFAULT_KEY_LONG_PRESS);
	// 키가 입력되고 처리된 적 없거나 시간이 충분히 지났을 경우
	bool IsInputedNotProcessedOrMoreThanTime(PERKeyValue key, bool pressed = true, float time = PER_DEFAULT_KEY_LONG_PRESS);

	void Update(float dTime);

private:
	void InitProcessedAndInputTime(PERKeyValue key, bool processed);

	std::atomic<bool> m_isMouseFixed;
	std::atomic<float> m_mousePosX, m_mousePosY;
	std::atomic<float> m_windowWidth, m_windowHeight;

	concurrency::concurrent_unordered_map<PERKeyValue, bool> m_KeyPressedMap;
	concurrency::concurrent_unordered_map<PERKeyValue, bool> m_KeyProcessedMap;
	concurrency::concurrent_unordered_map<PERKeyValue, float> m_KeyInputTimeMap;
};