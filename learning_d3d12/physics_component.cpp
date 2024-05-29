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
	XMMATRIX yRotate = Rotate(XMFLOAT3(0.f, 1.f, 0.f), rotation.y);
	XMFLOAT4X4 model = Matrix4x4::Identity();

	GetOwner()->SetModelTransform(Matrix4x4::Multiply(yRotate, model));

	GetOwner()->SetRotation(rotation);
}

XMMATRIX PhysicsComponent::Rotate(XMFLOAT3 axis, float angle)
{
	return XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(angle));
}
