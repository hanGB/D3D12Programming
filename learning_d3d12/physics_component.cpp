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
	PERComponent::Initialize();
}

void PhysicsComponent::Update(float dTime)
{
	// 이동
	// 월드 좌표계
	XMFLOAT3 worldForce = GetOwner()->GetWorldAsixForce();
	if (worldForce.x != 0.f || worldForce.y != 0.f || worldForce.z != 0.f) 
		MoveWorldAxis(worldForce.x * dTime, worldForce.y * dTime, worldForce.z * dTime);
	GetOwner()->SetWorldAsixForce(XMFLOAT3(0.f, 0.f, 0.0));

	// 로컬 좌표계
	XMFLOAT3 localForce = GetOwner()->GetLocalAsixForce();
	if (localForce.z != 0.f) MoveForward(localForce.z * dTime);
	if (localForce.x != 0.f) MoveStafe(localForce.x * dTime);
	if (localForce.y != 0.f) MoveUp(localForce.y * dTime);
	GetOwner()->SetLocalAsixForce(XMFLOAT3(0.f, 0.f, 0.f));

	// 회전
	XMFLOAT3 rotateForce = GetOwner()->GetRotateForce();
	Rotate(rotateForce.x * dTime, rotateForce.y * dTime, rotateForce.z * dTime);
	GetOwner()->SetRotateForce(XMFLOAT3(0.0f, 0.0f, 0.0f));

	if (GetNext()) dynamic_cast<PhysicsComponent*>(GetNext())->Update(dTime);
}

void PhysicsComponent::MoveWorldAxis(float xDistance, float yDistance, float zDistance)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	pos.x += xDistance;
	pos.y += yDistance;
	pos.z += zDistance;

	GetOwner()->SetPosition(pos);
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
