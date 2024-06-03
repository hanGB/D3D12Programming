#include "stdafx.h"
#include "player_input.h"
#include "per_object.h"

PlayerInput::PlayerInput()
{
}

PlayerInput::~PlayerInput()
{
}

void PlayerInput::Initialize()
{
	InputComponent::Initialize();
}

void PlayerInput::Update(PERController& controller, float dTime)
{
	XMFLOAT3 force = { 0.0f, 0.0f, 0.0f };

	if (controller.IsInputed(PERKeyValue::W)) force.z += 10.0f;
	if (controller.IsInputed(PERKeyValue::S)) force.z -= 10.0f;
	if (controller.IsInputed(PERKeyValue::A)) force.x -= 10.0f;
	if (controller.IsInputed(PERKeyValue::D)) force.x += 10.0f;
	if (controller.IsInputed(PERKeyValue::R)) force.y += 10.0f;
	if (controller.IsInputed(PERKeyValue::F)) force.y -= 10.0f;

	GetOwner()->SetLocalAsixForce(force);

	InputComponent::Update(controller, dTime);
}