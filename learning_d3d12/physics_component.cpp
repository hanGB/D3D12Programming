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
	XMFLOAT3 rotation = GetOwner()->GetRotation();

	rotation.y += dTime * 90.0f;

	GetOwner()->SetRotation(rotation);
}
