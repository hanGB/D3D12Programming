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
	MoveWorldAxis(worldForce.x * dTime, worldForce.y * dTime, worldForce.z * dTime);

	// 로컬 좌표계
	XMFLOAT3 localForce = GetOwner()->GetLocalAsixForce();
	MoveLocalAxis(localForce.x * dTime, localForce.y * dTime, localForce.z * dTime);

	// 회전
	XMFLOAT3 rotateForce = GetOwner()->GetRotateForce();
	Rotate(rotateForce.x * dTime, rotateForce.y * dTime, rotateForce.z * dTime);

	if (GetNext()) dynamic_cast<PhysicsComponent*>(GetNext())->Update(dTime);
}

void PhysicsComponent::SetGravity(XMFLOAT3 gravity)
{
	m_gravity = gravity;
}

void PhysicsComponent::SetFriction(float friction)
{
	m_friction = friction;
}

void PhysicsComponent::SetMaxVelocity(float xz, float y)
{
	m_fMaxVelocityXZ = xz;
	m_fMaxVelocityY = y;
}

void PhysicsComponent::MoveWorldAxis(float xDistance, float yDistance, float zDistance)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	pos.x += xDistance;
	pos.y += yDistance;
	pos.z += zDistance;

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::MoveLocalAxis(float xDistance, float yDistance, float zDistance)
{
	MoveForward(xDistance);
	MoveStafe(yDistance);
	MoveUp(zDistance);
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