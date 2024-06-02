#include "stdafx.h"
#include "input_component.h"

InputComponent::InputComponent()
{
}

InputComponent::~InputComponent()
{
}

void InputComponent::Initialize()
{
	PERComponent::Initialize();
}

void InputComponent::Update(PERController& controller, float dTime)
{
	if (GetNext()) dynamic_cast<InputComponent*>(GetNext())->Update(controller, dTime);
}
