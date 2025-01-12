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

	float movementForce = (float)c_MOVEMENT_FORCE;
	if (controller.IsInputed(PERKeyValue::W)) force.z += movementForce;
	if (controller.IsInputed(PERKeyValue::S)) force.z -= movementForce;
	if (controller.IsInputed(PERKeyValue::A)) force.x -= movementForce;
	if (controller.IsInputed(PERKeyValue::D)) force.x += movementForce;
	if (controller.IsInputed(PERKeyValue::R)) force.y += movementForce;
	if (controller.IsInputed(PERKeyValue::F)) force.y -= movementForce;

	GetOwner()->SetLocalAsixForce(force);

	// 마우스가 고정된 경우 카메라 회전 적용
	if (controller.GetIsMouseFixed()) 
	{
		short x, y;
		controller.GetMouseMoveDistance(&x, &y);

		XMFLOAT3 rotate = {(float)(y * c_ROTATION_SPEED), (float)(x * c_ROTATION_SPEED), 0.0f };

		GetOwner()->SetRotateForce(rotate);
	}


	InputComponent::Update(controller, dTime);
}
