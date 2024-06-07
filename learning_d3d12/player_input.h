#pragma once
#include "input_component.h"

class PlayerInput : public InputComponent {
public:
	PlayerInput();
	virtual ~PlayerInput();
	virtual void Initialize();

	virtual void Update(PERController& controller, float dTime);

private:
	static const int c_ROTATION_SPEED = 30;
	static const int c_MOVEMENT_FORCE = 500;
};