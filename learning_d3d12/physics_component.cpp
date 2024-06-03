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
	MoveWorldAxis(worldForce, dTime);

	// 로컬 좌표계
	XMFLOAT3 localForce = GetOwner()->GetLocalAsixForce();
	MoveLocalAxis(localForce, dTime);

	// 회전
	XMFLOAT3 rotateForce = GetOwner()->GetRotateForce();
	Rotate(rotateForce.x, rotateForce.y, rotateForce.z, dTime);

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

void PhysicsComponent::MoveWorldAxis(XMFLOAT3& shift, float dTime)
{
	XMFLOAT3 pos = GetOwner()->GetPosition();
	pos.x += shift.x * dTime;
	pos.y += shift.y * dTime;
	pos.z += shift.z * dTime;

	GetOwner()->SetPosition(pos);
}

void PhysicsComponent::MoveLocalAxis(XMFLOAT3& shift, float dTime)
{
	MoveForward(shift.x * dTime);
	MoveStafe(shift.y * dTime);
	MoveUp(shift.z * dTime);
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

void PhysicsComponent::Rotate(float pitch, float yaw, float roll, float dTime)
{
	XMFLOAT3 rotation = GetOwner()->GetRotation();
	rotation.x += pitch * dTime;
	rotation.y += yaw * dTime;
	rotation.z += roll * dTime;

	GetOwner()->SetRotation(rotation);
}
