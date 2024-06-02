#pragma once
#include "per_component.h"
#include "per_controller.h"

class InputComponent : public PERComponent {
public:
	InputComponent();
	virtual ~InputComponent();
	virtual void Initialize();

	virtual void Update(PERController& controller, float dTime);

private:
	
};