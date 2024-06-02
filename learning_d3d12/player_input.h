#pragma once
#include "input_component.h"

class PlayerInput : public InputComponent {
public:
	PlayerInput();
	virtual ~PlayerInput();
	virtual void Initialize();

	virtual void Update(PERController& controller, float dTime);

private:

};