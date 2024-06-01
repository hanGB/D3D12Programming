#include "stdafx.h"
#include "physics_component.h"
#include "per_object.h"

PhysicsComponent::PhysicsComponent()
{
}

PhysicsComponent::~PhysicsComponent()
{
}

void PhysicsComponent::Initialize()
{
}

void PhysicsComponent::Update(float dTime)
{
	Rotate(0.f, 90.f * dTime, 0.0f);
}

void PhysicsComponent::MoveStafe(float distance)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	XMFLOAT3 right = GetOwner()->GetRightVector();
	pos = Vector3::Add(pos, right, distance);

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::MoveUp(float distance)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	XMFLOAT3 up = GetOwner()->GetUpVector();
	pos = Vector3::Add(pos, up, distance);

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::MoveForward(float distance)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	XMFLOAT3 look = GetOwner()->GetLookVector();
	pos = Vector3::Add(pos, look, distance);

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::Rotate(float pitch, float yaw, float roll)
{
	XMFLOAT3 rotation = GetOwner()->GetRotation();
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;

	GetOwner()->SetRotation(rotation);
}
